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
#ifdef HAVE_CVS_IDENT
#ident "$Id: main.c,v 1.20.2.1 2006/06/27 01:37:14 steve Exp $"
#endif

# include "config.h"
# include "version.h"

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

# include  <stdio.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <unistd.h>
# include  <string.h>
# include  <ctype.h>
#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif
# include  "globals.h"

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern int getopt(int argc, char*argv[], const char*fmt);
extern int optind;
extern const char*optarg;
#endif

/*
 * Keep in source_list an array of pointers to file names. The array
 * is terminated by a pointer to null.
 */
static char**source_list = 0;
static unsigned source_cnt = 0;

void add_source_file(const char*name)
{
      if (source_list == 0) {
	    source_list = calloc(2, sizeof(char*));
	    source_list[0] = strdup(name);
	    source_list[1] = 0;
	    source_cnt = 1;
      } else {
	    source_list = realloc(source_list, sizeof(char*) * (source_cnt+2));
	    source_list[source_cnt+0] = strdup(name);
	    source_list[source_cnt+1] = 0;
	    source_cnt += 1;
      }
}

char**include_dir = 0;
unsigned include_cnt = 0;

int line_direct_flag = 0;

unsigned error_count = 0;
FILE *depend_file = NULL;

/*
 * This function reads from a file a list of file names. Each name
 * starts with the first non-space character, and ends with the last
 * non-space character. Spaces in the middle are OK.
 */
static int flist_read_names(const char*path)
{
      char line_buf[2048];

      FILE*fd = fopen(path, "r");
      if (fd == 0) {
	    fprintf(stderr, "%s: unable to open for reading.\n", path);
	    return 1;
      }

      while (fgets(line_buf, sizeof line_buf, fd) != 0) {
	    char*cp = line_buf + strspn(line_buf, " \t\r\b\f");
	    char*tail = cp + strlen(cp);
	    while (tail > cp) {
		  if (! isspace(tail[-1]))
			break;
		  tail -= 1;
		  tail[0] = 0;
	    }

	    if (cp < tail)
		  add_source_file(cp);
      }

      return 0;
}

int main(int argc, char*argv[])
{
      int opt, idx;
      const char*flist_path = 0;
      unsigned flag_errors = 0;
      const char*out_path = 0;
      const char*dep_path = NULL;
      FILE*out;

	/* Define preprocessor keywords that I plan to just pass. */
      define_macro("celldefine",          "`celldefine", 1);
      define_macro("default_nettype",     "`default_nettype", 1);
      define_macro("delay_mode_distributed", "`delay_mode_distributed", 1);
      define_macro("delay_mode_unit",     "`delay_mode_unit", 1);
      define_macro("delay_mode_path",     "`delay_mode_path", 1);
      define_macro("disable_portfaults",  "`disable_portfaults", 1);
      define_macro("enable_portfaults",   "`enable_portfaults", 1);
      define_macro("endcelldefine",       "`endcelldefine", 1);
      define_macro("endprotect",          "`endprotect", 1);
      define_macro("nosuppress_faults",   "`nosuppress_faults", 1);
      define_macro("nounconnected_drive", "`nounconnected_drive", 1);
      define_macro("protect",             "`protect", 1);
      define_macro("resetall",            "`resetall", 1);
      define_macro("suppress_faults",     "`suppress_faults", 1);
      define_macro("timescale",           "`timescale", 1);
      define_macro("unconnected_drive",   "`unconnected_drive", 1);
      define_macro("uselib",              "`uselib", 1);

      include_dir = malloc(sizeof(char*));
      include_dir[0] = strdup(".");
      include_cnt = 1;

      while ((opt = getopt(argc, argv, "D:f:I:K:LM:o:v")) != EOF) switch (opt) {

	  case 'D': {
		char*tmp = strdup(optarg);
		char*val = strchr(tmp, '=');
		if (val)
		      *val++ = 0;
		else
		      val = "1";

		define_macro(tmp, val, 0);
		free(tmp);
		break;
	  }

	  case 'f':
	    if (flist_path) {
		  fprintf(stderr, "%s: duplicate -f flag\n", argv[0]);
		  flag_errors += 1;
	    }
	    flist_path = optarg;
	    break;

	  case 'I':
	    include_dir = realloc(include_dir,
					  (include_cnt+1)*sizeof(char*));
	    include_dir[include_cnt] = strdup(optarg);
	    include_cnt += 1;
	    break;

	  case 'K': {
		char*buf = malloc(strlen(optarg) + 2);
		buf[0] = '`';
		strcpy(buf+1, optarg);
		define_macro(optarg, buf, 1);
		free(buf);
		break;
	  }

	  case 'L':
	    line_direct_flag = 1;
	    break;

	  case 'M':
	    if (dep_path) {
		  fprintf(stderr, "duplicate -M flag.\n");
	    } else {
		  dep_path = optarg;
	    }
	    break;

	  case 'o':
	    if (out_path) {
		  fprintf(stderr, "duplicate -o flag.\n");
	    } else {
		  out_path = optarg;
	    }
	    break;

	  case 'v':
	    fprintf(stderr, "Icarus Verilog Preprocessor version %s\n\n",
		    VERSION);
	    fprintf(stderr, "%s\n", COPYRIGHT);
	    fputs(NOTICE, stderr);
	    break;

	  default:
	    flag_errors += 1;
	    break;
      }

      if (flag_errors) {
	    fprintf(stderr, "\nUsage: %s [-v][-L][-I<dir>][-D<def>] <file>...\n"
		    "    -D<def> - Predefine a value.\n"
		    "    -f<fil> - Read the sources listed in the file\n"
		    "    -I<dir> - Add an include file search directory\n"
		    "    -K<def> - Define a keyword macro that I just pass\n"
		    "    -L      - Emit line number directives\n"
		    "    -M<fil> - Write dependencies to <fil>\n"
		    "    -o<fil> - Send the output to <fil>\n"
		    "    -v      - Print version information\n",
		    argv[0]);
	    return flag_errors;
      }

	/* Collect the file names on the command line in the source
	   file list, then if there is a file list, read more file
	   names from there. */
      for (idx = optind ;  idx < argc ;  idx += 1)
	    add_source_file(argv[idx]);


      if (flist_path) {
	    int rc = flist_read_names(flist_path);
	    if (rc != 0)
		  return rc;
      }

	/* Figure out what to use for an output file. Write to stdout
	   if no path is specified. */
      if (out_path) {
	    out = fopen(out_path, "w");
	    if (out == 0) {
		  perror(out_path);
		  exit(1);
	    }
      } else {
	    out = stdout;
      }

      if(dep_path) {
	      depend_file = fopen(dep_path, "w");
	      if (depend_file == 0) {
		  perror(dep_path);
		  exit(1);
	      }
      }

      if (source_cnt == 0) {
	    fprintf(stderr, "%s: No input files given.\n", argv[0]);
	    return 1;
      }

	/* Pass to the lexical analyzer the list of input file, and
	   start the parser. */
      reset_lexor(out, source_list);
      if (yyparse()) return -1;

      if(depend_file) {
	      fclose(depend_file);
      }

      return error_count;
}

/*
 * $Log: main.c,v $
 * Revision 1.20.2.1  2006/06/27 01:37:14  steve
 *  Fix const/non-const warnings.
 *
 * Revision 1.20  2004/09/10 00:15:45  steve
 *  Remove bad casts.
 *
 * Revision 1.19  2004/09/05 21:29:08  steve
 *  Better type safety.
 *
 * Revision 1.18  2004/02/15 18:03:30  steve
 *  Cleanup of warnings.
 *
 * Revision 1.17  2003/09/26 02:08:31  steve
 *  Detect missing endif markers.
 *
 * Revision 1.16  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.15  2002/04/04 05:26:13  steve
 *  Add dependency generation.
 *
 * Revision 1.14  2001/11/21 02:59:27  steve
 *  Remove diag print.
 *
 * Revision 1.13  2001/11/21 02:20:35  steve
 *  Pass list of file to ivlpp via temporary file.
 *
 * Revision 1.12  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.11  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.10  2001/06/23 18:41:02  steve
 *  Include stdlib.h
 *
 * Revision 1.9  2001/05/20 18:08:07  steve
 *  local declares if the header is missing.
 */

