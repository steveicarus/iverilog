# include  "version_base.h"
# include  "version_tag.h"
# include  "config.h"
# include  "parse_misc.h"
# include  "compile.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "statistics.h"
# include  "vvp_cleanup.h"
# include  "vvp_object.h"
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <unistd.h>
# include  <cassert>
#ifdef CHECK_WITH_VALGRIND
# include  <pthread.h>
#endif

#if defined(HAVE_SYS_RESOURCE_H)
#include <sys/time.h>
#include <sys/resource.h>
#endif // defined(HAVE_SYS_RESOURCE_H)

#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif

#if defined(__MINGW32__)
#include <windows.h>
#endif

using namespace std;

#if defined(HAVE_SYS_RESOURCE_H)
static void my_getrusage(struct rusage *a)
{
  getrusage(RUSAGE_SELF, a);

#if defined(LINUX)
  {
    FILE *statm;
    unsigned siz, rss, shd;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1)
      page_size = 0;
    statm = fopen("/proc/self/statm", "r");
    if (!statm)
    {
      perror("/proc/self/statm");
      return;
    }
    /* Given that these are in pages we'll limit the value to
     * what will fit in a 32 bit integer to prevent undefined
     * behavior in fscanf(). */
    if (3 == fscanf(statm, "%9u %9u %9u", &siz, &rss, &shd))
    {
      a->ru_maxrss = page_size * siz;
      a->ru_idrss = page_size * rss;
      a->ru_ixrss = page_size * shd;
    }
    fclose(statm);
  }
#endif
}

static void print_rusage(struct rusage *a, struct rusage *b)
{
  double delta = a->ru_utime.tv_sec + a->ru_utime.tv_usec / 1E6 + a->ru_stime.tv_sec + a->ru_stime.tv_usec / 1E6 - b->ru_utime.tv_sec - b->ru_utime.tv_usec / 1E6 - b->ru_stime.tv_sec - b->ru_stime.tv_usec / 1E6;

  vpi_mcd_printf(1,
                 " ... %G seconds,"
                 " %.1f/%.1f/%.1f KBytes size/rss/shared\n",
                 delta,
                 a->ru_maxrss / 1024.0,
                 (a->ru_idrss + a->ru_isrss) / 1024.0,
                 a->ru_ixrss / 1024.0);
}

#else // ! defined(HAVE_SYS_RESOURCE_H)

// Provide dummies
struct rusage
{
  int x;
};
inline static void my_getrusage(struct rusage *) {}
inline static void print_rusage(struct rusage *, struct rusage *){};

#endif // ! defined(HAVE_SYS_RESOURCE_H)

// ofstream debug_file;

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

static void final_cleanup()
{
      vvp_object::cleanup();

    /*
     * We only need to cleanup the memory if we are checking with valgrind.
     */
#ifdef CHECK_WITH_VALGRIND
    /* Clean up the file name table. */
      for (vector<const char*>::iterator cur = file_names.begin();
           cur != file_names.end() ; ++ cur ) {
        delete[] *cur;
      }
    /* Clear the static result buffer. */
      (void)need_result_buf(0, RBUF_DEL);
      codespace_delete();
      root_table_delete();
      def_table_delete();
      vpi_mcd_delete();
      dec_str_delete();
      modpath_delete();
      vpi_handle_delete();
      vpi_stack_delete();
      udp_defns_delete();
      island_delete();
      signal_pool_delete();
      vvp_net_pool_delete();
      ufunc_pool_delete();
#endif
    /*
     * Unload the VPI modules. This is essential for MinGW, to ensure
     * dump files are flushed before the main process terminates, as
     * the DLL termination code is called after all remaining open
     * files are automatically closed.
     */
      load_module_delete();

#ifdef CHECK_WITH_VALGRIND
      simulator_cb_delete();
    /* This is needed to prevent valgrind from complaining about
     * _dlerror_run() having a memory leak. */
// HERE: Is this portable? Does it break anything?
      pthread_exit(NULL);
#endif
}

static bool have_ivl_version = false;

extern unsigned module_cnt;
extern const char*module_tab[64];

extern void vpip_mcd_init(FILE *log);
extern void vvp_vpi_init(void);

void simulate(const char *const design_path) {
      struct rusage cycles[3];
      FILE *logfile = 0x0;
      extern void vpi_set_vlog_info(int, char**);
      extern bool stop_is_finish;
      extern int  stop_is_finish_exit_code;

      vpip_add_env_and_default_module_paths();

      vpip_mcd_init(logfile);
      vvp_vpi_init();
      compile_init();
      for (unsigned idx = 0 ;  idx < module_cnt ;  idx += 1)
        vpip_load_module(module_tab[idx]);
      compile_design(design_path);
      destroy_lexor();
      print_vpi_call_errors();

      if (!have_ivl_version) {
        if (verbose_flag) vpi_mcd_printf(1, "... ");
        vpi_mcd_printf(1, "Warning: vvp input file may not be correct "
                          "version!\n");
      }

      if (verbose_flag) {
        vpi_mcd_printf(1, "Compile cleanup...\n");
      }

      compile_cleanup();

      if (compile_errors > 0) {
        vpi_mcd_printf(1, "%s: Program not runnable, %u errors.\n",
            design_path, compile_errors);
        final_cleanup();
        return;
      }

      if (verbose_flag) {
        vpi_mcd_printf(1, " ... %8lu functors (net_fun pool=%zu bytes)\n",
               count_functors, vvp_net_fun_t::heap_total());
        vpi_mcd_printf(1, "           %8lu logic\n",  count_functors_logic);
        vpi_mcd_printf(1, "           %8lu bufif\n",  count_functors_bufif);
        vpi_mcd_printf(1, "           %8lu resolv\n",count_functors_resolv);
        vpi_mcd_printf(1, "           %8lu signals\n", count_functors_sig);
        vpi_mcd_printf(1, " ... %8lu filters (net_fil pool=%zu bytes)\n",
               count_filters, vvp_net_fil_t::heap_total());
        vpi_mcd_printf(1, " ... %8lu opcodes (%zu bytes)\n",
                       count_opcodes, size_opcodes);
        vpi_mcd_printf(1, " ... %8lu nets\n",     count_vpi_nets);
        vpi_mcd_printf(1, " ... %8lu vvp_nets (%zu bytes)\n",
               count_vvp_nets, size_vvp_nets);
        vpi_mcd_printf(1, " ... %8lu arrays (%lu words)\n",
               count_net_arrays, count_net_array_words);
        vpi_mcd_printf(1, " ... %8lu memories\n",
               count_var_arrays+count_real_arrays);
        vpi_mcd_printf(1, "           %8lu logic (%lu words)\n",
               count_var_arrays, count_var_array_words);
        vpi_mcd_printf(1, "           %8lu real (%lu words)\n",
               count_real_arrays, count_real_array_words);
        vpi_mcd_printf(1, " ... %8lu scopes\n",   count_vpi_scopes);
      }

      if (verbose_flag) {
        my_getrusage(cycles+1);
        print_rusage(cycles+1, cycles+0);
        vpi_mcd_printf(1, "Running ...\n");
      }


      schedule_simulate();

      if (verbose_flag) {
        my_getrusage(cycles+2);
        print_rusage(cycles+2, cycles+1);

        vpi_mcd_printf(1, "Event counts:\n");
        vpi_mcd_printf(1, "    %8lu time steps (pool=%lu)\n",
               count_time_events, count_time_pool());
        vpi_mcd_printf(1, "    %8lu thread schedule events\n",
            count_thread_events);
        vpi_mcd_printf(1, "    %8lu assign events\n",
            count_assign_events);
        vpi_mcd_printf(1, "             ...assign(vec4) pool=%lu\n",
               count_assign4_pool());
        vpi_mcd_printf(1, "             ...assign(vec8) pool=%lu\n",
               count_assign8_pool());
        vpi_mcd_printf(1, "             ...assign(real) pool=%lu\n",
               count_assign_real_pool());
        vpi_mcd_printf(1, "             ...assign(word) pool=%lu\n",
               count_assign_aword_pool());
        vpi_mcd_printf(1, "             ...assign(word/r) pool=%lu\n",
               count_assign_arword_pool());
        vpi_mcd_printf(1, "    %8lu other events (pool=%lu)\n",
               count_gen_events, count_gen_pool());
      }

      final_cleanup();

}
