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
#ident "$Id: main.cc,v 1.1 2001/03/11 00:29:38 steve Exp $"
#endif

# include  "config.h"
# include  "parse_misc.h"
# include  "compile.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <getopt.h>


int main(int argc, char*argv[])
{
      int opt;
      unsigned flag_errors = 0;
      const char*dump_path = 0;
      const char*design_path = 0;

      while ((opt = getopt(argc, argv, "D:")) != EOF) switch (opt) {
	  case 'D':
	    dump_path = optarg;
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

      compile_init();
      compile_design(design_path);
      compile_cleanup();

      if (dump_path) {
	    FILE*fd = fopen(dump_path, "w");
	    compile_dump(fd);
      }

      schedule_simulate();

      return 0;
}

/*
 * $Log: main.cc,v $
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */

