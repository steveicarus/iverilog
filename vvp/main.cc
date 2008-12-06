/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: main.cc,v 1.39.2.1 2007/02/16 23:29:17 steve Exp $"
#endif

# include  "config.h"
# include  "version.h"
# include  "parse_misc.h"
# include  "compile.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "statistics.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
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

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

bool verbose_flag = false;
bool version_flag = false;

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
      free(commit);

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

      free(ivl_ver);
      free(vvp_ver);
}

unsigned module_cnt = 0;
const char*module_tab[64];

extern void vpi_mcd_init(FILE *log);
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
	/* In the Windows world, we get the first module path
	   component relative the location where the binary lives. */
      { char path[4096], *s;
        GetModuleFileName(NULL,path,1024);
	  /* Get to the end.  Search back twice for backslashes */
	s = path + strlen(path);
	while (*s != '\\') s--; s--;
	while (*s != '\\') s--;
	strcpy(s,"\\lib\\ivl");
	vpip_module_path[0] = strdup(path);
      }
#endif

        /* For non-interactive runs we do not want to run the interactive
         * debugger, so make $stop just execute a $finish. */
      stop_is_finish = false;
      while ((opt = getopt(argc, argv, "+dhl:M:m:nvV")) != EOF) switch (opt) {
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
            fprintf(stderr, "Icarus Verilog runtime version " VERSION "\n\n");
            fprintf(stderr, "Copyright 2001-2006 Stephen Williams\n\n");
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

      vpi_mcd_init(logfile);

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

      if (int rc = compile_design(design_path))
	    return rc;

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
	    return compile_errors;
      }

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ... %8lu functors\n", count_functors);
	    vpi_mcd_printf(1, "           %8lu table\n",  count_functors_table);
	    vpi_mcd_printf(1, "           %8lu bufif\n",  count_functors_bufif);
	    vpi_mcd_printf(1, "           %8lu resolv\n",count_functors_resolv);
	    vpi_mcd_printf(1, "           %8lu variable\n", count_functors_var);
	    vpi_mcd_printf(1, " ... %8lu opcodes (%lu bytes)\n",
		    count_opcodes, (unsigned long)size_opcodes);
	    vpi_mcd_printf(1, " ... %8lu nets\n",     count_vpi_nets);
	    vpi_mcd_printf(1, " ... %8lu memories\n", count_vpi_memories);
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

	    vpi_mcd_printf(1, "Event counts: (event pool = %lu)\n",
		    count_event_pool);
	    vpi_mcd_printf(1, "    %8lu thread schedule events\n",
		    count_thread_events);
	    vpi_mcd_printf(1, "    %8lu propagation events\n",
		    count_prop_events);
	    vpi_mcd_printf(1, "    %8lu assign events\n",
		    count_assign_events);
	    vpi_mcd_printf(1, "    %8lu other events\n",
		    count_gen_events);
      }

      return 0;
}

/*
 * $Log: main.cc,v $
 * Revision 1.39.2.1  2007/02/16 23:29:17  steve
 *  Get page size from sysconf.
 *
 * Revision 1.39  2004/10/04 01:10:59  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.38  2003/06/25 04:04:19  steve
 *  Fix mingw portability problems.
 *
 * Revision 1.37  2003/06/13 19:51:08  steve
 *  Include verbose messages in log output.
 *
 * Revision 1.36  2003/05/15 16:51:09  steve
 *  Arrange for mcd id=00_00_00_01 to go to stdout
 *  as well as a user specified log file, set log
 *  file to buffer lines.
 *
 *  Add vpi_flush function, and clear up some cunfused
 *  return codes from other vpi functions.
 *
 *  Adjust $display and vcd/lxt messages to use the
 *  standard output/log file.
 *
 * Revision 1.35  2003/03/13 04:36:57  steve
 *  Remove the obsolete functor delete functions.
 *
 * Revision 1.34  2003/02/07 02:45:05  steve
 *  Mke getopt ignore options after the file name.
 *
 * Revision 1.33  2003/01/18 23:55:35  steve
 *  Add a means to clear the module search path.
 *
 * Revision 1.32  2003/01/06 23:57:26  steve
 *  Schedule wait lists of threads as a single event,
 *  to save on events. Also, improve efficiency of
 *  event_s allocation. Add some event statistics to
 *  get an idea where performance is really going.
 *
 * Revision 1.31  2002/09/18 03:34:07  steve
 *  printf size warning.
 *
 * Revision 1.30  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.29  2002/07/15 00:21:42  steve
 *  Fix initialization of symbol table string heap.
 *
 * Revision 1.28  2002/07/05 20:08:44  steve
 *  Count different types of functors.
 *
 * Revision 1.27  2002/07/05 17:14:15  steve
 *  Names of vpi objects allocated as vpip_strings.
 *
 * Revision 1.26  2002/07/05 03:47:06  steve
 *  Track opcode memory space.
 *
 * Revision 1.25  2002/07/05 02:50:58  steve
 *  Remove the vpi object symbol table after compile.
 *
 * Revision 1.24  2002/04/12 02:44:02  steve
 *  Formally define extended arguments to vvp.
 *
 * Revision 1.23  2002/03/01 05:43:14  steve
 *  Add cleanup to verbose messages.
 *
 * Revision 1.22  2002/01/09 03:15:23  steve
 *  Add vpi_get_vlog_info support.
 *
 * Revision 1.21  2001/10/20 01:03:42  steve
 *  Print memory usage information if requested (Stephan Boettcher)
 */

