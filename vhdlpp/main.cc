
const char COPYRIGHT[] =
      "Copyright (c) 2011-2024 Stephen Williams (steve@icarus.com)\n"
      "Copyright CERN 2012 / Stephen Williams (steve@icarus.com)";
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
# include  "vhdlpp_config.h"
# include  "version_base.h"
# include  "version_tag.h"

using namespace std;

/*
 * Usage:  vhdlpp [flags] sourcefile...
 * Flags:
 *
 **  -D <token>
 *     This activates various sorts of debugging aids. The <token>
 *     specifies which debugging aid to activate. Valid tokens are:
 *
 *     yydebug | no-yydebug
 *        Enable (disable) debug prints from the bison parser
 *
 *     libraries=<path>
 *        Enable debugging of library support by dumping library
 *        information to the file named <path>.
 *
 *     elaboration=<path>
 *        Enable debugging of elaboration by dumping elaboration
 *        process information to the file named <path>.
 *
 *     entities=<path>
 *        Enable debugging of elaborated entities by writing the
 *        elaboration results to the file named <path>.
 *
 **  -v
 *     Verbose operation. Display verbose non-debug information.
 *
 **  -V
 *     Version. Print the version of this binary.
 *
 **  -w <path>
 *     Work path. This sets the path to the working library
 *     directory. I write into that directory files for packages that
 *     I declare, and I read from that directory packages that are
 *     already declared. The default path is "ivl_vhdl_work".
 */

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

# include  "compiler.h"
# include  "library.h"
# include  "std_funcs.h"
# include  "std_types.h"
# include  "parse_api.h"
# include  "vtype.h"
# include  <fstream>
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <cerrno>
# include  <limits>
#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif
# include  <sys/stat.h>
// MinGW only supports mkdir() with a path. If this stops working because
// we need to use _mkdir() for mingw-w32 and mkdir() for mingw-w64 look
// at using the autoconf AX_FUNC_MKDIR macro to figure this all out.
#if defined(__MINGW32__)
# include <io.h>
# define mkdir(path, mode) mkdir(path)
#endif


bool verbose_flag = false;
  // Where to dump design entities
const char*dump_design_entities_path = 0;
const char*dump_libraries_path = 0;
const char*debug_log_path = 0;

bool debug_elaboration = false;
ofstream debug_log_file;

extern void dump_libraries(ostream&file);
extern void parser_cleanup();

static void process_debug_token(const char*word)
{
      if (strcmp(word, "yydebug") == 0) {
	    yydebug = 1;
      } else if (strcmp(word, "no-yydebug") == 0) {
	    yydebug = 0;
      } else if (strncmp(word, "entities=", 9) == 0) {
	    dump_design_entities_path = strdup(word+9);
      } else if (strncmp(word, "libraries=", 10) == 0) {
	    dump_libraries_path = strdup(word+10);
      } else if (strncmp(word, "log=", 4) == 0) {
	    debug_log_path = strdup(word+4);
      } else if (strcmp(word, "elaboration") == 0) {
	    debug_elaboration = true;
      }
}

int main(int argc, char*argv[])
{
      int opt;
      int rc;
      const char*work_path = "ivl_vhdl_work";

      while ( (opt=getopt(argc, argv, "D:L:vVw:")) != EOF) switch (opt) {

	  case 'D':
	    process_debug_token(optarg);
	    break;

	  case 'L':
	    library_add_directory(optarg);
	    break;

	  case 'v':
	    fprintf(stderr, "Icarus Verilog VHDL Parse version "
		    VERSION " (" VERSION_TAG ")\n\n");
	    fprintf(stderr, "%s\n\n", COPYRIGHT);
	    fputs(NOTICE, stderr);
	    verbose_flag = true;
	    break;

	  case 'V':
	    fprintf(stdout, "Icarus Verilog VHDL Parse version "
		    VERSION " (" VERSION_TAG ")\n\n");
	    fprintf(stdout, "%s\n\n", COPYRIGHT);
	    fputs(NOTICE, stdout);
	    break;

	  case 'w':
	    work_path = optarg;
	    break;
      }

      if (debug_log_path) {
	    debug_log_file.open(debug_log_path);
      }

      if ( (mkdir(work_path, 0777)) < 0 ) {
	    if (errno != EEXIST) {
		  fprintf(stderr, "Icarus Verilog VHDL unable to create work directory %s, errno=%d\n", work_path, errno);
		  return -1;
	    }
	    struct stat stat_buf;
	    rc = stat(work_path, &stat_buf);

	    if (rc || !S_ISDIR(stat_buf.st_mode)) {
		  fprintf(stderr, "Icarus Verilog VHDL work path `%s' is not a directory.\n", work_path);
		  return -1;
	    }
      }

      std::cout.precision(std::numeric_limits<double>::digits10);
      library_set_work_path(work_path);

      preload_global_types();
      preload_std_funcs();

      int errors = 0;

      for (int idx = optind ; idx < argc ; idx += 1) {
	    parse_errors = 0;
	    parse_sorrys = 0;
	    rc = parse_source_file(argv[idx], perm_string());
	    if (rc < 0)
		  return 1;

	    if (verbose_flag)
		  fprintf(stderr, "parse_source_file() returns %d, parse_errors=%d, parse_sorrys=%d\n", rc, parse_errors, parse_sorrys);

	    if (parse_errors > 0) {
		  fprintf(stderr, "Encountered %d errors parsing %s\n", parse_errors, argv[idx]);
	    }
	    if (parse_sorrys > 0) {
		  fprintf(stderr, "Encountered %d unsupported constructs parsing %s\n", parse_sorrys, argv[idx]);
	    }

	    if (parse_errors || parse_sorrys) {
		  errors += parse_errors;
		  errors += parse_sorrys;
		  break;
	    }
      }

      if (dump_libraries_path) {
	    ofstream file(dump_libraries_path);
	    dump_libraries(file);
      }

      if (dump_design_entities_path) {
	    ofstream file(dump_design_entities_path);
	    dump_design_entities(file);
      }

      if (errors > 0) {
	    parser_cleanup();
	    return 2;
      }

      errors = elaborate_entities();
      if (errors > 0) {
	    fprintf(stderr, "%d errors elaborating design.\n", errors);
	    parser_cleanup();
	    return 3;
      }

      errors = elaborate_libraries();
      if (errors > 0) {
	    fprintf(stderr, "%d errors elaborating libraries.\n", errors);
	    parser_cleanup();
	    return 4;
      }

      emit_std_types(cout);

      errors = emit_packages();
      if (errors > 0) {
	    fprintf(stderr, "%d errors emitting packages.\n", errors);
	    parser_cleanup();
	    return 5;
      }

      errors = emit_entities();
      if (errors > 0) {
	    fprintf(stderr, "%d errors emitting design.\n", errors);
	    parser_cleanup();
	    return 6;
      }

      parser_cleanup();
      return 0;
}
