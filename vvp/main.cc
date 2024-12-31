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
# include  "version_tag.h"
# include  "config.h"
# include  "compile.h"
# include  <cstdio>
# include  <cstdlib>

#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif

#if defined(__MINGW32__)
# include  <windows.h>
#endif

#include "libvvp.h"

using namespace std;

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

bool version_flag = false;

unsigned module_cnt = 0;
const char*module_tab[64];

int main(int argc, char*argv[])
{
      int opt;
      unsigned flag_errors = 0;
      const char *logfile_name = 0x0;

      while ((opt = getopt(argc, argv, "+hil:M:m:nNqsvV")) != EOF) switch (opt) {
         case 'h':
           fprintf(stderr,
                   "Usage: vvp [options] input-file [+plusargs...]\n"
                   "Options:\n"
                   " -h             Print this help message.\n"
                   " -i             Interactive mode (unbuffered stdio).\n"
                   " -l file        Logfile, '-' for <stderr>\n"
                   " -M path        VPI module directory\n"
		   " -M -           Clear VPI module path\n"
                   " -m module      Load vpi module.\n"
		   " -n             Non-interactive ($stop = $finish).\n"
                   " -N             Same as -n, but exit code is 1 instead of 0\n"
		   " -q             Quiet mode (suppress output on MCD bit 0).\n"
		   " -s             $stop right away.\n"
                   " -v             Verbose progress messages.\n"
                   " -V             Print the version information.\n" );
           exit(0);
	  case 'i':
	    setvbuf(stdout, 0, _IONBF, 0);
	    break;
	  case 'l':
	    logfile_name = optarg;
	    break;
	  case 'M':
	    if (strcmp(optarg,"-") == 0) {
		  vpip_clear_module_paths();
	    } else {
		  vpip_add_module_path(optarg);
	    }
	    break;
	  case 'm':
	    module_tab[module_cnt++] = optarg;
	    break;
	  case 'n':
	    vvp_set_stop_is_finish(true, 0);
	    break;
          case 'N':
            vvp_set_stop_is_finish(true, 1);
            break;
	  case 'q':
	    vvp_set_quiet_flag(true);
	    break;
	  case 's':
	    schedule_stop(0);
	    break;
	  case 'v':
	    vvp_set_verbose_flag(true);
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
	    fprintf(stderr, "%s\n\n", COPYRIGHT);
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

      vvp_init(logfile_name, argc - optind, argv + optind);

      for (unsigned idx = 0 ;  idx < module_cnt ;  idx += 1)
	    vpip_load_module(module_tab[idx]);

      return vvp_run(argv[optind]);
}
