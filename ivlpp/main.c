const char COPYRIGHT[] =
  "Copyright (c) 1999-2024 Stephen Williams (steve@icarus.com)";
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

# include  "config.h"
# include  "version_base.h"
# include  "version_tag.h"

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
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;

# include  <stdio.h>
# include  <stdlib.h>
# include  <unistd.h>
# include  <string.h>
# include  <ctype.h>
#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif
# include  "globals.h"
# include  "ivl_alloc.h"

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern int getopt(int argc, char*argv[], const char*fmt);
extern int optind;
extern const char*optarg;
#endif
/* Path to the dependency file, if there is one. */
char *dep_path = NULL;
/* Dependency file output mode */
char dep_mode = 'a';
/* verbose flag */
int verbose_flag = 0;
/* Path to vhdlpp */
char *vhdlpp_path = 0;
/* vhdlpp work directory */
char *vhdlpp_work = 0;

char**vhdlpp_libdir = 0;
unsigned vhdlpp_libdir_cnt = 0;
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

int relative_include = 0;

int line_direct_flag = 0;

unsigned error_count = 0;
FILE *depend_file = NULL;

/* Should we warn about macro redefinitions? */
int warn_redef = 0;
int warn_redef_all = 0;

static int flist_read_flags(const char*path)
{
      char line_buf[2048];
      FILE*fd = fopen(path, "r");
      if (fd == 0) {
	    fprintf(stderr, "%s: unable to open for reading.\n", path);
	    return -1;
      }

      while (fgets(line_buf, sizeof line_buf, fd) != 0) {
	      /* Skip leading white space. */
	    char*cp = line_buf + strspn(line_buf, " \t\r\b\f");
	      /* Remove trailing white space. */
	    char*tail = cp + strlen(cp);
	    char*arg;

	    while (tail > cp) {
		  if (! isspace((int)tail[-1]))
			break;
		  tail -= 1;
		  tail[0] = 0;
	    }

	      /* Skip empty lines */
	    if (*cp == 0)
		  continue;
	      /* Skip comment lines */
	    if (cp[0] == '#')
		  continue;

	      /* The arg points to the argument to the keyword. */
	    arg = strchr(cp, ':');
	    if (arg) *arg++ = 0;

	    if (strcmp(cp,"D") == 0) {
		  char*val = strchr(arg, '=');
		  const char *valo = "1";
		  if (val) {
			*val++ = 0;
			valo = val;
		  }

		  define_macro(arg, valo, 0, 0);

	    } else if (strcmp(cp,"I") == 0) {
		  include_dir = realloc(include_dir,
					(include_cnt+1)*sizeof(char*));
		  include_dir[include_cnt] = strdup(arg);
		  include_cnt += 1;

	    } else if (strcmp(cp,"keyword") == 0) {
		  char*buf = malloc(strlen(arg) + 2);
		  buf[0] = '`';
		  strcpy(buf+1, optarg);
		  define_macro(optarg, buf, 1, 0);
		  free(buf);

	    } else if ((strcmp(cp,"Ma") == 0)
                   ||  (strcmp(cp,"Mi") == 0)
                   ||  (strcmp(cp,"Mm") == 0)
                   ||  (strcmp(cp,"Mp") == 0)) {
		  if (dep_path) {
			fprintf(stderr, "duplicate -M flag.\n");
                  } else {
                        dep_mode = cp[1];
			dep_path = strdup(arg);
		  }

	    } else if (strcmp(cp,"relative include") == 0) {
		  if (strcmp(arg, "true") == 0) {
			relative_include = 1;
		  } else {
			relative_include = 0;
		  }

	    } else if (strcmp(cp,"vhdlpp") == 0) {
		  if (vhdlpp_path) {
			fprintf(stderr, "Ignore multiple vhdlpp flags\n");
		  } else {
			vhdlpp_path = strdup(arg);
		  }

	    } else if (strcmp(cp,"vhdlpp-work") == 0) {
		  if (vhdlpp_work) {
			fprintf(stderr, "Ignore duplicate vhdlpp-work flags\n");
		  } else {
			vhdlpp_work = strdup(arg);
		  }

	    } else if (strcmp(cp,"vhdlpp-libdir") == 0) {
		  vhdlpp_libdir = realloc(vhdlpp_libdir,
					  (vhdlpp_libdir_cnt+1)*sizeof(char*));
		  vhdlpp_libdir[vhdlpp_libdir_cnt] = strdup(arg);
		  vhdlpp_libdir_cnt += 1;

	    } else {
		  fprintf(stderr, "%s: Invalid keyword %s\n", path, cp);
	    }
      }

      fclose(fd);
      return 0;
}

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
		  if (! isspace((int)tail[-1]))
			break;
		  tail -= 1;
		  tail[0] = 0;
	    }

	    if (cp < tail)
		  add_source_file(cp);
      }

      fclose(fd);
      return 0;
}

int main(int argc, char*argv[])
{
      int opt, idx;
      unsigned lp;
      const char*flist_path = 0;
      unsigned flag_errors = 0;
      char*out_path = 0;
      FILE*out;
      char*precomp_out_path = 0;
      FILE*precomp_out = NULL;

	/* Define preprocessor keywords that I plan to just pass. */
	/* From 1364-2005 Chapter 19. */
      define_macro("begin_keywords",          "`begin_keywords", 1, 0);
      define_macro("celldefine",              "`celldefine", 1, 0);
      define_macro("default_nettype",         "`default_nettype", 1, 0);
      define_macro("end_keywords",            "`end_keywords", 1, 0);
      define_macro("endcelldefine",           "`endcelldefine", 1, 0);
      define_macro("line",                    "`line", 1, 0);
      define_macro("nounconnected_drive",     "`nounconnected_drive", 1, 0);
      define_macro("pragma",                  "`pragma", 1, 0);
      define_macro("resetall",                "`resetall", 1, 0);
      define_macro("timescale",               "`timescale", 1, 0);
      define_macro("unconnected_drive",       "`unconnected_drive", 1, 0);

	/* From 1364-2005 Annex D. */
      define_macro("default_decay_time",      "`default_decay_time", 1, 0);
      define_macro("default_trireg_strength", "`default_trireg_strength", 1, 0);
      define_macro("delay_mode_distributed",  "`delay_mode_distributed", 1, 0);
      define_macro("delay_mode_path",         "`delay_mode_path", 1, 0);
      define_macro("delay_mode_unit",         "`delay_mode_unit", 1, 0);
      define_macro("delay_mode_zero",         "`delay_mode_zero", 1, 0);

	/* From other places. */
      define_macro("disable_portfaults",      "`disable_portfaults", 1, 0);
      define_macro("enable_portfaults",       "`enable_portfaults", 1, 0);
      define_macro("endprotect",              "`endprotect", 1, 0);
      define_macro("nosuppress_faults",       "`nosuppress_faults", 1, 0);
      define_macro("protect",                 "`protect", 1, 0);
      define_macro("suppress_faults",         "`suppress_faults", 1, 0);
      define_macro("uselib",                  "`uselib", 1, 0);

      include_cnt = 2;
      include_dir = malloc(include_cnt*sizeof(char*));
      include_dir[0] = 0;  /* 0 is reserved for the current files path. */
      include_dir[1] = strdup(".");

      while ((opt=getopt(argc, argv, "F:f:K:Lo:p:P:vVW:")) != EOF) switch (opt) {

	  case 'F':
	    flist_read_flags(optarg);
	    break;

	  case 'f':
	    if (flist_path) {
		  fprintf(stderr, "%s: duplicate -f flag\n", argv[0]);
		  flag_errors += 1;
	    }
	    flist_path = optarg;
	    break;

	  case 'K': {
		char*buf = malloc(strlen(optarg) + 2);
		buf[0] = '`';
		strcpy(buf+1, optarg);
		define_macro(optarg, buf, 1, 0);
		free(buf);
		break;
	  }

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

	  case 'p':
	    if (precomp_out_path) {
		  fprintf(stderr, "duplicate -p flag.\n");
	    } else {
		  precomp_out_path = optarg;
	    }
	    break;

	  case 'P': {
		FILE*src = fopen(optarg, "rb");
		if (src == 0) {
		      perror(optarg);
		      exit(1);
		}
		load_precompiled_defines(src);
		fclose(src);
		break;
	  }

	  case 'W':
	    if (strcmp(optarg, "redef-all") == 0) {
		  warn_redef_all = 1;
		  warn_redef = 1;
	    } else if (strcmp(optarg, "redef-chg") == 0) {
		  warn_redef = 1;
	    }
	    break;

	  case 'v':
	    fprintf(stderr, "Icarus Verilog Preprocessor version "
		    VERSION " (" VERSION_TAG ")\n\n");
	    fprintf(stderr, "%s\n\n", COPYRIGHT);
	    fputs(NOTICE, stderr);
	    verbose_flag = 1;
	    break;

	  case 'V':
	    fprintf(stdout, "Icarus Verilog Preprocessor version "
		    VERSION " (" VERSION_TAG ")\n\n");
	    fprintf(stdout, "%s\n\n", COPYRIGHT);
	    fputs(NOTICE, stdout);
	    return 0;

	  default:
	    flag_errors += 1;
	    break;
      }

      if (flag_errors) {
	    fprintf(stderr, "\nUsage: %s [-v][-L][-F<fil>][-f<fil>] <file>...\n"
		    "    -F<fil> - Get defines and includes from file\n"
		    "    -f<fil> - Read the sources listed in the file\n"
		    "    -K<def> - Define a keyword macro that I just pass\n"
		    "    -L      - Emit line number directives\n"
		    "    -o<fil> - Send the output to <fil>\n"
		    "    -p<fil> - Write precompiled defines to <fil>\n"
		    "    -P<fil> - Read precompiled defines from <fil>\n"
		    "    -v      - Verbose\n"
		    "    -V      - Print version information and quit\n"
		    "    -W<cat> - Enable extra ivlpp warning category:\n"
		    "               o redef-all - all macro redefinitions\n"
		    "               o redef-chg - macro definition changes\n",
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

      if (precomp_out_path) {
	    precomp_out = fopen(precomp_out_path, "wb");
	    if (precomp_out == 0) {
		  if (out_path) fclose(out);
		  perror(precomp_out_path);
		  exit(1);
	    }
      }

      if (dep_path) {
	      depend_file = fopen(dep_path, "a");
	      if (depend_file == 0) {
		  if (out_path) fclose(out);
		  if (precomp_out) fclose(precomp_out);
		  perror(dep_path);
		  exit(1);
	      }
      }

      if (source_cnt == 0) {
	    fprintf(stderr, "%s: No input files given.\n", argv[0]);
	    if (out_path) fclose(out);
	    if (depend_file) fclose(depend_file);
	    if (precomp_out) fclose(precomp_out);
	    return 1;
      }

      if (vhdlpp_work == 0) {
	    vhdlpp_work = strdup("ivl_vhdl_work");
      }

	/* Pass to the lexical analyzer the list of input file, and
	   start scanning. */
      reset_lexor(out, source_list);
      if (yylex()) {
	    if (out_path) fclose(out);
	    if (depend_file) fclose(depend_file);
	    if (precomp_out) fclose(precomp_out);
	    return -1;
      }
      destroy_lexor();

      if (depend_file) fclose(depend_file);

      if (precomp_out) {
	    dump_precompiled_defines(precomp_out);
	    fclose(precomp_out);
      }

      if (error_count) {
	    if (out_path) fprintf(stderr, "%s: had (%u) errors.\n", argv[0], error_count);
	    fprintf(out, "// Icarus preprocessor had (%u) errors.\n", error_count);
      }

      if (out_path) fclose(out);

	/* Free the source and include directory lists. */
      for (lp = 0; lp < source_cnt; lp += 1) {
	    free(source_list[lp]);
      }
      free(source_list);
      for (lp = 0; lp < include_cnt; lp += 1) {
	    free(include_dir[lp]);
      }
      free(include_dir);

	/* Free the VHDL library directories, the path and work directory. */
      for (lp = 0; lp < vhdlpp_libdir_cnt; lp += 1) {
	    free(vhdlpp_libdir[lp]);
      }
      free(vhdlpp_libdir);
      free(vhdlpp_path);
      free(vhdlpp_work);

      free_macros();

      return error_count;
}
