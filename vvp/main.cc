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
#if !defined(WINNT)
#ident "$Id: main.cc,v 1.23 2002/03/01 05:43:14 steve Exp $"
#endif

# include  "config.h"
# include  "debug.h"
# include  "parse_misc.h"
# include  "compile.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <unistd.h>

#if defined(HAVE_SYS_RESOURCE_H)
# include  <sys/time.h>
# include  <sys/resource.h>
# if defined(LINUX)
#  include <asm/page.h> 
# endif
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


#if defined(HAVE_SYS_RESOURCE_H)
static void my_getrusage(struct rusage *a)
{
      getrusage(RUSAGE_SELF, a);

#     if defined(LINUX)
      {
	    FILE *statm;
	    unsigned siz, rss, shd;
	    statm = fopen("/proc/self/statm", "r");
	    if (!statm) {
		  perror("/proc/self/statm");
		  return;
	    }
	    if (3<=fscanf(statm, "%u%u%u", &siz, &rss, &shd)) {
		  a->ru_maxrss = PAGE_SIZE * siz;
		  a->ru_idrss  = PAGE_SIZE * rss;
		  a->ru_ixrss  = PAGE_SIZE * shd;
	    }
	    fclose(statm);
      }
#     endif
}

static void print_rusage(FILE *f, struct rusage *a, struct rusage *b)
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

      fprintf(f,
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
inline static void print_rusage(FILE *, struct rusage *, struct rusage *){};

#endif // ! defined(HAVE_SYS_RESOURCE_H)


unsigned module_cnt = 0;
const char*module_tab[64];

extern void vpi_mcd_init(FILE *log);
extern void vvp_vpi_init(void);

int main(int argc, char*argv[])
{
      int opt;
      unsigned flag_errors = 0;
      const char*design_path = 0;
      bool debug_flag = false;
      bool verbose_flag = false;
      struct rusage cycles[3];
      const char *logfile_name = 0x0;
      FILE *logfile = 0x0;
      extern void vpi_set_vlog_info(int, char**);

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

      while ((opt = getopt(argc, argv, "dhl:M:m:v")) != EOF) switch (opt) {
	  case 'd':
	    debug_flag = true;
	    break;
         case 'h':
           fprintf(stderr,
                   "Usage: vvp [options] input-file\n"
                   "Options:\n"
#if defined(WITH_DEBUG)
                   " -d             Enter the debugger.\n"
#endif
                   " -h             Print this help message.\n"
                   " -l file        Logfile, '-' for <stderr>\n"
                   " -M path        VPI module directory\n"
                   " -m module      Load vpi module.\n"
                   " -v             Verbose progress messages.\n" );
           exit(0);
	  case 'l':
	    logfile_name = optarg;
	    break;
	  case 'M':
	    vpip_module_path[vpip_module_path_cnt++] = optarg;
	    break;
	  case 'm':
	    module_tab[module_cnt++] = optarg;
	    break;
	  case 'v':
	    verbose_flag = true;
	    break;
	  default:
	    flag_errors += 1;
      }

      if (flag_errors)
	    return flag_errors;

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
	    else 
		  logfile = fopen(logfile_name, "w");
	    if (!logfile) {
		  perror(logfile_name);
		  exit(1);
	    }
      }

      if (verbose_flag) {
	    my_getrusage(cycles+0);
	    fprintf(stderr, "Compiling VVP ...\n");
	    if (logfile && logfile != stderr)
		  fprintf(logfile, "Compiling VVP ...\n");
      }

      vpi_mcd_init(logfile);

      vvp_vpi_init();

      vpi_set_vlog_info(argc, argv);

      compile_init();

      for (unsigned idx = 0 ;  idx < module_cnt ;  idx += 1)
	    vpip_load_module(module_tab[idx]);

      if (int rc = compile_design(design_path))
	    return rc;

      if (verbose_flag) {
	    fprintf(stderr, "Compile cleanup...\n");
      }

      compile_cleanup();

      if (compile_errors > 0) {
	    fprintf(stderr, "%s: Program not runnable, %u errors.\n",
		    design_path, compile_errors);
	    if (logfile && logfile != stderr)
		  fprintf(logfile, "%s: Program not runnable, %u errors.\n",
			  design_path, compile_errors);
	    return compile_errors;
      }

      if (verbose_flag) {
	    my_getrusage(cycles+1);
	    print_rusage(stderr, cycles+1, cycles+0);
	    fprintf(stderr, "Running ...\n");
	    if (logfile && logfile != stderr) {
		  print_rusage(logfile, cycles+1, cycles+0);
		  fprintf(logfile, "Running ...\n");
	    }
      }
       
#if defined(WITH_DEBUG)
      if (debug_flag)
	    breakpoint();
#endif

      schedule_simulate();

      if (verbose_flag) {
	    my_getrusage(cycles+2);
	    print_rusage(stderr, cycles+2, cycles+1);
	    if (logfile && logfile != stderr)
		  print_rusage(logfile, cycles+2, cycles+1);
      }

      return 0;
}

/*
 * $Log: main.cc,v $
 * Revision 1.23  2002/03/01 05:43:14  steve
 *  Add cleanup to verbose messages.
 *
 * Revision 1.22  2002/01/09 03:15:23  steve
 *  Add vpi_get_vlog_info support.
 *
 * Revision 1.21  2001/10/20 01:03:42  steve
 *  Print memory usage information if requested (Stephan Boettcher)
 *
 * Revision 1.20  2001/07/30 02:44:05  steve
 *  Cleanup defines and types for mingw compile.
 *
 * Revision 1.19  2001/07/26 03:13:51  steve
 *  Make the -M flag add module search paths.
 *
 * Revision 1.18  2001/07/21 21:18:55  steve
 *  Add the -h flag for help. (Stephan Boettcher)
 *
 * Revision 1.17  2001/07/16 18:40:19  steve
 *  Add a stdlog output for vvp, and vvp options
 *  to direct them around. (Stephan Boettcher.)
 *
 * Revision 1.16  2001/06/23 18:26:26  steve
 *  Add the %shiftl/i0 instruction.
 *
 * Revision 1.15  2001/06/12 03:53:11  steve
 *  Change the VPI call process so that loaded .vpi modules
 *  use a function table instead of implicit binding.
 *
 * Revision 1.14  2001/05/20 17:34:53  steve
 *  declare getopt by hand in mingw32 compile.
 *
 * Revision 1.13  2001/05/12 20:38:06  steve
 *  A resolver that understands some simple strengths.
 *
 * Revision 1.12  2001/05/11 03:26:31  steve
 *  No entry breakpoint if debug is compiled out.
 *
 * Revision 1.11  2001/05/11 02:06:14  steve
 *  Add the --enable-vvp-debug option to the configure
 *  script of vvp, and detect getopt.h.
 *
 * Revision 1.10  2001/05/09 04:23:19  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.9  2001/05/08 23:32:26  steve
 *  Add to the debugger the ability to view and
 *  break on functors.
 *
 *  Add strengths to functors at compile time,
 *  and Make functors pass their strengths as they
 *  propagate their output.
 *
 * Revision 1.8  2001/04/04 04:33:08  steve
 *  Take vector form as parameters to vpi_call.
 *
 * Revision 1.7  2001/03/23 02:40:22  steve
 *  Add the :module header statement.
 *
 * Revision 1.6  2001/03/22 22:38:14  steve
 *  Detect undefined system tasks at compile time.
 *
 * Revision 1.5  2001/03/22 21:26:54  steve
 *  Compile in a default VPI module dir.
 *
 * Revision 1.4  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.3  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 * Revision 1.2  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */

