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
#ident "$Id: main.cc,v 1.20 2001/07/30 02:44:05 steve Exp $"
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
#if defined(HAVE_TIMES)
# include  <unistd.h>
# include  <sys/times.h>
#endif
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

#if defined(HAVE_TIMES)
static double cycles_diff(struct tms *a, struct tms *b)
{
      clock_t aa = a->tms_utime 
	    +      a->tms_stime 
	    +      a->tms_cutime 
	    +      a->tms_cstime;

      clock_t bb = b->tms_utime 
	    +      b->tms_stime 
	    +      b->tms_cutime 
	    +      b->tms_cstime;

      return (aa-bb)/(double)sysconf(_SC_CLK_TCK);
}
#else // ! defined(HAVE_TIMES)
// Provide dummies
struct tms { int x; };
inline static void times(struct tms *) { }
inline static double cycles_diff(struct tms *a, struct tms *b) { return 0; }
#endif // ! defined(HAVE_TIMES)


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
      struct tms cycles[3];
      const char *logfile_name = 0x0;
      FILE *logfile = 0x0;

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
	    times(cycles+0);
	    fprintf(stderr, "Compiling VVP ...\n");
           if (logfile && logfile != stderr)
                 fprintf(logfile, "Compiling VVP ...\n");
      }

      vpi_mcd_init(logfile);
      
      vvp_vpi_init();

      compile_init();

      for (unsigned idx = 0 ;  idx < module_cnt ;  idx += 1)
	    vpip_load_module(module_tab[idx]);

      if (int rc = compile_design(design_path))
	    return rc;
      compile_cleanup();

      if (verbose_flag) {
	    times(cycles+1);
	    fprintf(stderr, 
		    " ... %G seconds\n"
		    "Running ...\n", 
		    cycles_diff(cycles+1, cycles+0));
           if (logfile && logfile != stderr)
                 fprintf(logfile, 
                         " ... %G seconds\n"
                         "Running ...\n", 
                         cycles_diff(cycles+1, cycles+0));
      }
       
      if (compile_errors > 0) {
	    fprintf(stderr, "%s: Program not runnable, %u errors.\n",
		    design_path, compile_errors);
           if (logfile && logfile != stderr)
                 fprintf(logfile, "%s: Program not runnable, %u errors.\n",
                         design_path, compile_errors);
	    return compile_errors;
      }

#if defined(WITH_DEBUG)
      if (debug_flag)
	    breakpoint();
#endif

      schedule_simulate();

      if (verbose_flag) {
	    times(cycles+2);
	    fprintf(stderr, " ... %G seconds\n", 
		    cycles_diff(cycles+2, cycles+1));
           if (logfile && logfile != stderr)
                 fprintf(logfile, " ... %G seconds\n", 
                         cycles_diff(cycles+2, cycles+1));
      }

      return 0;
}

/*
 * $Log: main.cc,v $
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

