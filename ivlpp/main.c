const char COPYRIGHT[] =
          "Copyright (c) 1999 Stephen Williams (steve@icarus.com)";
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#if !defined(WINNT)
#ident "$Id: main.c,v 1.5 2000/06/30 15:49:44 steve Exp $"
#endif

const char NOTICE[] =
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
"  You should have received a copy of the GNU General Public License\n"
"  along with this program; if not, write to the Free Software\n"
"  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA\n"
;

const char VERSION[] = "$Name:  $ $State: Exp $";

# include  <stdio.h>
# include  <malloc.h>
# include  <unistd.h>
# include  <string.h>
#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif
# include  "globals.h"

char**include_dir = 0;
unsigned include_cnt = 0;

int line_direct_flag = 0;

unsigned error_count = 0;

int main(int argc, char*argv[])
{
      int opt;
      unsigned flag_errors = 0;
      char*out_path = 0;
      FILE*out;

      include_dir = malloc(sizeof(char*));
      include_dir[0] = strdup(".");
      include_cnt = 1;

      while ((opt = getopt(argc, argv, "D:I:Lo:v")) != EOF) switch (opt) {

	  case 'D': {
		char*tmp = strdup(optarg);
		char*val = strchr(tmp, '=');
		if (val)
		      *val++ = 0;
		else
		      val = "1";

		define_macro(tmp, val);
		free(tmp);
		break;
	  }

	  case 'I':
	    include_dir = realloc(include_dir, (include_cnt+1)*sizeof(char*));
	    include_dir[include_cnt] = strdup(optarg);
	    include_cnt += 1;
	    break;

	  case 'L':
	    line_direct_flag = 1;
	    break;

	  case 'o':
	    if (out_path) {
		  fprintf(stderr, "duplicate -o flag.\n");
	    } else {
		  out_path = optarg;
	    }
	    break;

	  case 'v':
	    fprintf(stderr, "Icarus Verilog Preprocessor version %s\n",
		    VERSION);
	    fprintf(stderr, "%s\n", COPYRIGHT);
	    fputs(NOTICE, stderr);
	    break;

	  default:
	    flag_errors += 1;
	    break;
      }

      if (optind == argc)
	    flag_errors += 1;

      if (flag_errors) {
	    fprintf(stderr, "\nUsage: %s [-v][-L][-I<dir>][-D<def>] <file>...\n"
		    "    -D<def> - Predefine a value.\n"
		    "    -I<dir> - Add an include file search directory\n"
		    "    -L      - Emit line number directives\n"
		    "    -o<fil> - Send the output to <fil>\n"
		    "    -v      - Print version information\n",
		    argv[0]);
	    return flag_errors;
      }

      if (out_path) {
	    out = fopen(out_path, "w");
	    if (out == 0) {
		  perror(out_path);
		  exit(1);
	    }
      } else {
	    out = stdout;
      }

      if (argv[optind] == 0) {
	    fprintf(stderr, "%s: No input files given.\n", argv[0]);
	    return 1;
      }

      reset_lexor(out, argv+optind);

      if (yyparse()) return -1;

      return error_count;
}

/*
 * $Log: main.c,v $
 * Revision 1.5  2000/06/30 15:49:44  steve
 *  Handle errors from parser slightly differently.
 *
 * Revision 1.4  1999/11/29 17:02:21  steve
 *  include getopt if present.
 *
 * Revision 1.3  1999/09/05 22:33:18  steve
 *  Take multiple source files on the command line.
 *
 * Revision 1.2  1999/07/03 20:03:47  steve
 *  Add include path and line directives.
 *
 * Revision 1.1  1999/07/03 17:24:11  steve
 *  Add a preprocessor.
 *
 */

