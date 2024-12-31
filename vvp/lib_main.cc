const char COPYRIGHT[] =
  "Copyright (c) 2001-2024 Stephen Williams (steve@icarus.com)";
/*
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "version_base.h"
# include  "config.h"
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
# include  <sys/time.h>
# include  <sys/resource.h>
#endif // defined(HAVE_SYS_RESOURCE_H)

#if defined(__MINGW32__)
# include  <windows.h>
#endif

#include "libvvp.h"

using namespace std;

ofstream debug_file;

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

bool verbose_flag = false;
static int vvp_return_value = 0;
static int vvp_used = 0;

void vvp_set_stop_is_finish(bool flag, int exit_code)
{
      extern bool stop_is_finish;
      extern int  stop_is_finish_exit_code;

      stop_is_finish = flag;
      stop_is_finish_exit_code = exit_code;
}

void vvp_set_quiet_flag(bool flag)
{
      vpip_mcd0_disable = flag;
}

void vvp_set_verbose_flag(bool flag)
{
      verbose_flag = flag;
}

void vpip_set_return_value(int value)
{
      vvp_return_value = value;
}

static char log_buffer[4096];

#if defined(HAVE_SYS_RESOURCE_H)
static void my_getrusage(struct rusage *a)
{
      getrusage(RUSAGE_SELF, a);

#     if defined(LINUX)
      {
	    FILE *statm;
	    unsigned siz, rss, shd;
	    long page_size = sysconf(_SC_PAGESIZE);
	    if (page_size==-1) page_size=0;
	    statm = fopen("/proc/self/statm", "r");
	    if (!statm) {
		  perror("/proc/self/statm");
		  return;
	    }
	      /* Given that these are in pages we'll limit the value to
	       * what will fit in a 32 bit integer to prevent undefined
	       * behavior in fscanf(). */
	    if (3 == fscanf(statm, "%9u %9u %9u", &siz, &rss, &shd)) {
		  a->ru_maxrss = page_size * siz;
		  a->ru_idrss  = page_size * rss;
		  a->ru_ixrss  = page_size * shd;
	    }
	    fclose(statm);
      }
#     endif
}

static void print_rusage(struct rusage *a, struct rusage *b)
{
      double delta = a->ru_utime.tv_sec
	    +        a->ru_utime.tv_usec/1E6
	    +        a->ru_stime.tv_sec
	    +        a->ru_stime.tv_usec/1E6
	    -        b->ru_utime.tv_sec
	    -        b->ru_utime.tv_usec/1E6
	    -        b->ru_stime.tv_sec
	    -        b->ru_stime.tv_usec/1E6
	    ;

      vpi_mcd_printf(1,
	      " ... %G seconds,"
	      " %.1f/%.1f/%.1f KBytes size/rss/shared\n",
	      delta,
	      a->ru_maxrss/1024.0,
	      (a->ru_idrss+a->ru_isrss)/1024.0,
	      a->ru_ixrss/1024.0 );
}

#else // ! defined(HAVE_SYS_RESOURCE_H)

// Provide dummies
struct rusage { int x; };
inline static void my_getrusage(struct rusage *) { }
inline static void print_rusage(struct rusage *, struct rusage *){};

#endif // ! defined(HAVE_SYS_RESOURCE_H)

static bool have_ivl_version = false;
/*
 * Verify that the input file has a compatible version.
 */
void verify_version(char*ivl_ver, char*commit)
{
      have_ivl_version = true;

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ... VVP file version %s", ivl_ver);
	    if (commit) vpi_mcd_printf(1, " %s", commit);
	    vpi_mcd_printf(1, "\n");
      }
      delete[] commit;

      int file_major, file_minor, file_minor2;
      char file_extra[128];

	// Old style format: 0.<major>.<minor> <extra>
	// This also catches a potential new-new format that has
	// another sub-minor number.
      file_extra[0] = 0;
      int rc = sscanf(ivl_ver, "%d.%d.%d %127s", &file_major, &file_minor, &file_minor2, file_extra);

	// If it wasn't the old style format, try the new format:
	// <major>.<minor> <extra>
      if (rc == 2) {
	    file_extra[0] = 0;
	    rc = sscanf(ivl_ver, "%d.%d %127s", &file_major, &file_minor, file_extra);
	    assert((rc == 2) || (rc == 3));
	    file_minor2 = 0;
      }
      delete[] ivl_ver;

	// If this was the old format, the file_major will be 0. In
	// this case it is not really what we meant, so convert to the
	// new format.
      if (file_major == 0) {
	    file_major = file_minor;
	    file_minor = file_minor2;
	    file_minor2 = 0;
      }

      if (VERSION_MAJOR != file_major) {
	    vpi_mcd_printf(1, "Error: VVP input file %d.%d can not "
			   "be run with run time version %s\n",
			   file_major, file_minor, VERSION);
	    exit(1);
      }

      if (VERSION_MINOR < file_minor) {
	    vpi_mcd_printf(1, "Warning: VVP input file sub version %d.%d"
			   " is greater than the run time version %s.\n",
			   file_major, file_minor, VERSION);
      }
}

int vpip_delay_selection = _vpiDelaySelTypical;
void set_delay_selection(const char* sel)
{
      if (strcmp("TYPICAL", sel) == 0) {
	    vpip_delay_selection = _vpiDelaySelTypical;
      } else if (strcmp("MINIMUM", sel) == 0) {
	    vpip_delay_selection = _vpiDelaySelMinimum;
      } else if (strcmp("MAXIMUM", sel) == 0) {
	    vpip_delay_selection = _vpiDelaySelMaximum;
      } else {
	    vpi_mcd_printf(1, "Error: Unknown delay selection \"%s\"!", sel);
	    exit(1);
      }
      delete[] sel;
}

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

extern void vpip_mcd_init(FILE *log);
extern void vvp_vpi_init(void);

static void report_used(void)
{
  fprintf(stderr,
	  "This VVP simulation has already run and can not be reused\n");
}

void vvp_init(const char *logfile_name, int argc, char*argv[])
{
      struct rusage cycle;
      FILE *logfile = 0x0;
      extern void vpi_set_vlog_info(int, char**);

      if (vvp_used++) {
          report_used();
          return;
      }

      if( ::getenv("VVP_WAIT_FOR_DEBUGGER") != 0 ) {
          fprintf( stderr, "Waiting for debugger...\n");
          bool debugger_release = false;
          while( !debugger_release )  {
#if defined(__MINGW32__)
              Sleep(1000);
#else
              sleep(1);
#endif
        }
      }

      vpip_add_env_and_default_module_paths();

	/* If the VVP_DEBUG variable is set, then it contains the path
	   to the vvp debug file. Open it for output. */

      if (char*path = getenv("VVP_DEBUG")) {
	    debug_file.open(path, ios::out);
      }

	/* This is needed to get the MCD I/O routines ready for
	   anything. It is done early because it is plausible that the
	   compile might affect it, and it is cheap to do. */

      if (logfile_name) {
	    if (!strcmp(logfile_name, "-"))
		  logfile = stderr;
	    else {
		  logfile = fopen(logfile_name, "w");
		  if (!logfile) {
		        perror(logfile_name);
		        exit(1);
		  }
		  setvbuf(logfile, log_buffer, _IOLBF, sizeof(log_buffer));
	    }
      }

      vpip_mcd_init(logfile);

      if (verbose_flag) {
	    my_getrusage(&cycle);
	    vpi_mcd_printf(1, "Compiling VVP ...\n");
      }

      vvp_vpi_init();

	/* Make the extended arguments available to the simulation. */
      vpi_set_vlog_info(argc, argv);

      compile_init();
}

int vvp_run(const char *design_path)
{
      struct rusage cycles[3];
      int ret_cd;

      if (vvp_used++ != 1) {
          if (vvp_used == 1)
              fprintf(stderr, "vvp_init() has not been called\n");
          else
              report_used();
          return 1;
      }
      ++vvp_used;

      ret_cd = compile_design(design_path);
      destroy_lexor();
      print_vpi_call_errors();
      if (ret_cd) return ret_cd;

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
	    return compile_errors;
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

      return vvp_return_value;
}
