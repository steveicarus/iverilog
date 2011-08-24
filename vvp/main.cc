/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
 *
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "version_base.h"
# include  "version_tag.h"
# include  "config.h"
# include  "parse_misc.h"
# include  "compile.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "statistics.h"
# include  "vvp_cleanup.h"
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <unistd.h>

#if defined(HAVE_SYS_RESOURCE_H)
# include  <sys/time.h>
# include  <sys/resource.h>
#endif // defined(HAVE_SYS_RESOURCE_H)

#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif

#if defined(__MINGW32__)
# include  <windows.h>
#endif

ofstream debug_file;

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

bool verbose_flag = false;
bool version_flag = false;
static int vvp_return_value = 0;

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
	    if (3<=fscanf(statm, "%u%u%u", &siz, &rss, &shd)) {
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

      char*vvp_ver = strdup(VERSION);
      char *vp, *ip;

	/* Check the major/minor version. */
      ip = strrchr(ivl_ver, '.');
      *ip = '\0';
      vp = strrchr(vvp_ver, '.');
      *vp = '\0';
      if (strcmp(ivl_ver, vvp_ver) != 0) {
	    vpi_mcd_printf(1, "Error: VVP input file version %s can not "
	                      "be run with run time version %s!\n",
	                      ivl_ver, vvp_ver);
	    exit(1);
      }

	/* Check that the sub-version is compatible. */
      ip += 1;
      vp += 1;
      int ivl_sv, vvp_sv;
      if (strcmp(ip, "devel") == 0) {
	    ivl_sv = -1;
      } else {
	    int res = sscanf(ip, "%d", &ivl_sv);
	    assert(res == 1);
      }
      if (strcmp(vp, "devel") == 0) {
	    vvp_sv = -1;
      } else {
	    int res = sscanf(vp, "%d", &vvp_sv);
	    assert(res == 1);
      }
      if (ivl_sv > vvp_sv) {
	    if (verbose_flag) vpi_mcd_printf(1, " ... ");
	    vpi_mcd_printf(1, "Warning: VVP input file sub-version %s is "
	                      "greater than the run time sub-version %s!\n",
	                      ip, vp);
      }

      delete[] ivl_ver;
      free(vvp_ver);
}

static void final_cleanup()
{
	/*
	 * We only need to cleanup the memory if we are checking with valgrind.
	 */
#ifdef CHECK_WITH_VALGRIND
	/* Clean up the file name table. */
      for (vector<const char*>::iterator cur = file_names.begin();
           cur != file_names.end() ; cur++) {
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
}

unsigned module_cnt = 0;
const char*module_tab[64];

extern void vpip_mcd_init(FILE *log);
extern void vvp_vpi_init(void);

int main(int argc, char*argv[])
{
      int opt;
      unsigned flag_errors = 0;
      const char*design_path = 0;
      struct rusage cycles[3];
      const char *logfile_name = 0x0;
      FILE *logfile = 0x0;
      extern void vpi_set_vlog_info(int, char**);
      extern bool stop_is_finish;

#ifdef __MINGW32__
	/* Calculate the module path from the path to the command.
	   This is necessary because of the installation process on
	   Windows. Mostly, it is those darn drive letters, but oh
	   well. We know the command path is formed like this:

		D:\iverilog\bin\iverilog.exe

	   The IVL_ROOT in a Windows installation is the path:

		D:\iverilog\lib\ivl$(suffix)

	   so we chop the file name and the last directory by
	   turning the last two \ characters to null. Then we append
	   the lib\ivl$(suffix) to finish. */
      char *s;
      char basepath[4096], tmp[4096];
      GetModuleFileName(NULL, tmp, sizeof tmp);
	/* Convert to a short name to remove any embedded spaces. */
      GetShortPathName(tmp, basepath, sizeof basepath);
      s = strrchr(basepath, '\\');
      if (s) *s = 0;
      else {
	    fprintf(stderr, "%s: Missing first \\ in exe path!\n", argv[0]);
	    exit(1);
      }
      s = strrchr(basepath, '\\');
      if (s) *s = 0;
      else {
	    fprintf(stderr, "%s: Missing second \\ in exe path!\n", argv[0]);
	    exit(1);
      }
      strcat(s, "\\lib\\ivl" IVL_SUFFIX);
      vpip_module_path[0] = strdup(basepath);
#endif

        /* For non-interactive runs we do not want to run the interactive
         * debugger, so make $stop just execute a $finish. */
      stop_is_finish = false;
      while ((opt = getopt(argc, argv, "+hl:M:m:nsvV")) != EOF) switch (opt) {
         case 'h':
           fprintf(stderr,
                   "Usage: vvp [options] input-file [+plusargs...]\n"
                   "Options:\n"
                   " -h             Print this help message.\n"
                   " -l file        Logfile, '-' for <stderr>\n"
                   " -M path        VPI module directory\n"
		   " -M -           Clear VPI module path\n"
                   " -m module      Load vpi module.\n"
		   " -n             Non-interactive ($stop = $finish).\n"
		   " -s             $stop right away.\n"
                   " -v             Verbose progress messages.\n"
                   " -V             Print the version information.\n" );
           exit(0);
	  case 'l':
	    logfile_name = optarg;
	    break;
	  case 'M':
	    if (strcmp(optarg,"-") == 0) {
		  vpip_module_path_cnt = 0;
		  vpip_module_path[0] = 0;
	    } else {
		  vpip_module_path[vpip_module_path_cnt++] = optarg;
	    }
	    break;
	  case 'm':
	    module_tab[module_cnt++] = optarg;
	    break;
	  case 'n':
	    stop_is_finish = true;
	    break;
	  case 's':
	    schedule_stop(0);
	    break;
	  case 'v':
	    verbose_flag = true;
	    break;
	  case 'V':
	    version_flag = true;
	    break;
	  default:
	    flag_errors += 1;
      }

      if (flag_errors)
	    return flag_errors;

      if (version_flag) {
	    fprintf(stderr, "Icarus Verilog runtime version " VERSION " ("
	                    VERSION_TAG ")\n\n");
	    fprintf(stderr, "Copyright 1998-2010 Stephen Williams\n\n");
	    fprintf(stderr,
"  This program is free software; you can redistribute it and/or modify\n"
"  it under the terms of the GNU General Public License as published by\n"
"  the Free Software Foundation; either version 2 of the License, or\n"
"  (at your option) any later version.\n"
"\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"  GNU General Public License for more details.\n"
"\n"
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n\n"
);
	    return 0;
      }

      if (optind == argc) {
	    fprintf(stderr, "%s: no input file.\n", argv[0]);
	    return -1;
      }

	/* If the VVP_DEBUG variable is set, then it contains the path
	   to the vvp debug file. Open it for output. */

      if (char*path = getenv("VVP_DEBUG")) {
	    debug_file.open(path, ios::out);
      }

      design_path = argv[optind];

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
	    my_getrusage(cycles+0);
	    vpi_mcd_printf(1, "Compiling VVP ...\n");
      }

      vvp_vpi_init();

	/* Make the extended arguments available to the simulation. */
      vpi_set_vlog_info(argc-optind, argv+optind);

      compile_init();

      for (unsigned idx = 0 ;  idx < module_cnt ;  idx += 1)
	    vpip_load_module(module_tab[idx]);

      int ret_cd = compile_design(design_path);
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
#ifdef __MINGW32__  /* MinGW does not know about z. */
	    vpi_mcd_printf(1, " ... %8lu functors (net_fun pool=%u bytes)\n",
#else
	    vpi_mcd_printf(1, " ... %8lu functors (net_fun pool=%zu bytes)\n",
#endif
			   count_functors, size_vvp_net_funs);
	    vpi_mcd_printf(1, "           %8lu logic\n",  count_functors_logic);
	    vpi_mcd_printf(1, "           %8lu bufif\n",  count_functors_bufif);
	    vpi_mcd_printf(1, "           %8lu resolv\n",count_functors_resolv);
	    vpi_mcd_printf(1, "           %8lu signals\n", count_functors_sig);
#ifdef __MINGW32__  /* MinGW does not know about z. */
	    vpi_mcd_printf(1, " ... %8lu opcodes (%u bytes)\n",
#else
	    vpi_mcd_printf(1, " ... %8lu opcodes (%zu bytes)\n",
#endif
	                   count_opcodes, size_opcodes);
	    vpi_mcd_printf(1, " ... %8lu nets\n",     count_vpi_nets);
#ifdef __MINGW32__  /* MinGW does not know about z. */
	    vpi_mcd_printf(1, " ... %8lu vvp_nets (%u bytes)\n",
#else
	    vpi_mcd_printf(1, " ... %8lu vvp_nets (%zu bytes)\n",
#endif
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
	    vpi_mcd_printf(1, "    %8lu other events (pool=%lu)\n",
			   count_gen_events, count_gen_pool());
      }

      final_cleanup();

      return vvp_return_value;
}
