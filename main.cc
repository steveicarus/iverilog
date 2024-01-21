const char COPYRIGHT[] =
  "Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)";
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

# include "config.h"
# include "version_base.h"
# include "version_tag.h"

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

# include  <cstdio>
# include  <iostream>
# include  <fstream>
# include  <queue>
# include  <cstring>
# include  <list>
# include  <map>
# include  <unistd.h>
# include  <cstdlib>
#if defined(HAVE_TIMES)
# include  <sys/times.h>
#endif
#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif
# include  "pform.h"
# include  "parse_api.h"
# include  "PGenerate.h"
# include  "netlist.h"
# include  "target.h"
# include  "compiler.h"
# include  "discipline.h"
# include  "t-dll.h"

using namespace std;

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

#if defined(__CYGWIN__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

#if defined(TRAP_SIGINT_FOR_DEBUG)
/*
 * This is a debugging aid. Do not compile it in general, but leave it
 * here for those days when I need the ability to cleanly exit on a
 * signal interrupt.
*/
# include  <csignal>
static void signals_handler(int sig)
{
      fprintf(stderr, "Exit on signal %d\n", sig);
      exit(1);
}
#endif

# include  "ivl_alloc.h"

/* Count errors detected in flag processing. */
unsigned flag_errors = 0;
static unsigned long pre_process_fail_count = 0;

const char*basedir = strdup(".");

/*
 * These are the language support control flags. These support which
 * language features (the generation) to support. The generation_flag
 * is a major mode, and the gn_* flags control specific sub-features.
 */
generation_t generation_flag = GN_DEFAULT;
bool gn_icarus_misc_flag = true;
bool gn_cadence_types_flag = true;
bool gn_specify_blocks_flag = true;
bool gn_interconnect_flag = true;
bool gn_supported_assertions_flag = true;
bool gn_unsupported_assertions_flag = true;
bool gn_io_range_error_flag = true;
bool gn_strict_ca_eval_flag = false;
bool gn_strict_expr_width_flag = false;
bool gn_shared_loop_index_flag = true;
bool gn_verilog_ams_flag = false;

/*
 * For some generations we allow a system function to be called
 * as a task and only print a warning message. The default for
 * this is that it is a run time error.
 */
ivl_sfunc_as_task_t def_sfunc_as_task = IVL_SFUNC_AS_TASK_ERROR;

map<string,const char*> flags;
char*vpi_module_list = 0;
void add_vpi_module(const char*name)
{
      if (vpi_module_list == 0) {
	    vpi_module_list = strdup(name);

      } else {
	    char*tmp = (char*)realloc(vpi_module_list,
				      strlen(vpi_module_list)
				      + strlen(name)
				      + 2);
	    strcat(tmp, ",");
	    strcat(tmp, name);
	    vpi_module_list = tmp;
      }
      flags["VPI_MODULE_LIST"] = vpi_module_list;
      load_vpi_module(name);
}

map<perm_string,unsigned> missing_modules;
map<perm_string,bool> library_file_map;

vector<perm_string> source_files;

list<const char*> library_suff;

list<perm_string> roots;

char*ivlpp_string = 0;

char depfile_mode = 'a';
char* depfile_name = NULL;
FILE *depend_file = NULL;

/*
 * These are the warning enable flags.
 */
bool warn_implicit  = false;
bool warn_implicit_dimensions = false;
bool warn_timescale = false;
bool warn_portbinding = false;
bool warn_inf_loop = false;
bool warn_ob_select = false;
bool warn_sens_entire_vec = false;
bool warn_sens_entire_arr = false;
bool warn_anachronisms = false;
bool warn_floating_nets = false;

/*
 * Ignore errors about missing modules
 */
bool ignore_missing_modules = false;

/*
 * Debug message class flags.
 */
bool debug_scopes = false;
bool debug_eval_tree = false;
bool debug_elaborate = false;
bool debug_emit = false;
bool debug_synth2 = false;
bool debug_optimizer = false;

/*
 * Compilation control flags.
 */
bool separate_compilation = false;

/*
 * Optimization control flags.
 */
unsigned opt_const_func = 0;

/*
 * Miscellaneous flags.
 */
bool disable_virtual_pins = false;
unsigned long array_size_limit = 16777216;  // Minimum required by IEEE-1364?
unsigned recursive_mod_limit = 10;
bool disable_concatz_generation = false;

/*
 * Verbose messages enabled.
 */
bool verbose_flag = false;

unsigned integer_width = 32;

/*
 * Width limit for unsized expressions.
 */
unsigned width_cap = 65536;

int def_ts_units = 0;
int def_ts_prec = 0;

/*
 * Keep a heap of identifier strings that I encounter. This is a more
 * efficient way to allocate those strings.
 */
StringHeapLex lex_strings;

StringHeapLex filename_strings;

StringHeapLex bits_strings;

/*
 * In library searches, Windows file names are never case sensitive.
 */
#if defined(__MINGW32__)
const bool CASE_SENSITIVE = false;
#else
const bool CASE_SENSITIVE = true;
#endif

/*
 * Are we doing synthesis?
 */
bool synthesis = false;

extern void cprop(Design*des);
extern void exposenodes(Design*des);
extern void synth(Design*des);
extern void synth2(Design*des);
extern void syn_rules(Design*des);
extern void nodangle(Design*des);

typedef void (*net_func)(Design*);
static struct net_func_map {
      const char*name;
      void (*func)(Design*);
} func_table[] = {
      { "cprop",       &cprop },
      { "exposenodes", &exposenodes },
      { "nodangle",    &nodangle },
      { "synth",       &synth },
      { "synth2",      &synth2 },
      { "syn-rules",   &syn_rules },
      { 0, 0 }
};

queue<net_func> net_func_queue;

net_func name_to_net_func(const string&name)
{
      for (unsigned idx = 0 ;  func_table[idx].name ;  idx += 1)
	    if (name == func_table[idx].name)
		  return func_table[idx].func;

      return 0;
}

const char *net_func_to_name(const net_func func)
{
      for (unsigned idx = 0 ;  func_table[idx].name ;  idx += 1)
	    if (func == func_table[idx].func)
		  return func_table[idx].name;

      return "This cannot happen";
}

static void process_generation_flag(const char*gen)
{
      if (strcmp(gen,"1") == 0) { // FIXME: Deprecated for 1995
	    generation_flag = GN_VER1995;

      } else if (strcmp(gen,"2") == 0) { // FIXME: Deprecated for 2001
	    generation_flag = GN_VER2001;

      } else if (strcmp(gen,"2x") == 0) { // FIXME: Deprecated for 2001
	    generation_flag = GN_VER2001;
	    gn_icarus_misc_flag = true;

      } else if (strcmp(gen,"1995") == 0) {
	    generation_flag = GN_VER1995;

      } else if (strcmp(gen,"2001") == 0) {
	    generation_flag = GN_VER2001;

      } else if (strcmp(gen,"2001-noconfig") == 0) {
	    generation_flag = GN_VER2001_NOCONFIG;

      } else if (strcmp(gen,"2005") == 0) {
	    generation_flag = GN_VER2005;

      } else if (strcmp(gen,"2005-sv") == 0) {
	    generation_flag = GN_VER2005_SV;

      } else if (strcmp(gen,"2009") == 0) {
	    generation_flag = GN_VER2009;

      } else if (strcmp(gen,"2012") == 0) {
	    generation_flag = GN_VER2012;

      } else if (strcmp(gen,"icarus-misc") == 0) {
	    gn_icarus_misc_flag = true;

      } else if (strcmp(gen,"no-icarus-misc") == 0) {
	    gn_icarus_misc_flag = false;

      } else if (strcmp(gen,"xtypes") == 0) {
	    gn_cadence_types_flag = true;

      } else if (strcmp(gen,"no-xtypes") == 0) {
	    gn_cadence_types_flag = false;

      } else if (strcmp(gen,"specify") == 0) {
	    gn_specify_blocks_flag = true;

      } else if (strcmp(gen,"no-specify") == 0) {
	    gn_specify_blocks_flag = false;

      } else if (strcmp(gen,"interconnect") == 0) {
	    gn_interconnect_flag = true;

      } else if (strcmp(gen,"no-interconnect") == 0) {
	    gn_interconnect_flag = false;

      } else if (strcmp(gen,"assertions") == 0) {
	    gn_supported_assertions_flag = true;
	    gn_unsupported_assertions_flag = true;

      } else if (strcmp(gen,"supported-assertions") == 0) {
	    gn_supported_assertions_flag = true;
	    gn_unsupported_assertions_flag = false;

      } else if (strcmp(gen,"no-assertions") == 0) {
	    gn_supported_assertions_flag = false;
	    gn_unsupported_assertions_flag = false;

      } else if (strcmp(gen,"verilog-ams") == 0) {
	    gn_verilog_ams_flag = true;

      } else if (strcmp(gen,"no-verilog-ams") == 0) {
	    gn_verilog_ams_flag = false;

      } else if (strcmp(gen,"io-range-error") == 0) {
	    gn_io_range_error_flag = true;

      } else if (strcmp(gen,"no-io-range-error") == 0) {
	    gn_io_range_error_flag = false;

      } else if (strcmp(gen,"strict-ca-eval") == 0) {
	    gn_strict_ca_eval_flag = true;

      } else if (strcmp(gen,"no-strict-ca-eval") == 0) {
	    gn_strict_ca_eval_flag = false;

      } else if (strcmp(gen,"strict-expr-width") == 0) {
	    gn_strict_expr_width_flag = true;

      } else if (strcmp(gen,"no-strict-expr-width") == 0) {
	    gn_strict_expr_width_flag = false;

      } else if (strcmp(gen,"shared-loop-index") == 0) {
	    gn_shared_loop_index_flag = true;

      } else if (strcmp(gen,"no-shared-loop-index") == 0) {
	    gn_shared_loop_index_flag = false;

	  } else {
      }
}

static void parm_to_flagmap(const string&flag)
{
      string key;
      const char*value;
      unsigned off = flag.find('=');
      if (off > flag.size()) {
	    key = flag;
	    value = strdup("");

      } else {
	    key = flag.substr(0, off);
	    value = strdup(flag.substr(off+1).c_str());
      }

      flags[key] = value;
}

static void find_module_mention(map<perm_string,bool>&check_map, Module*m);
static void find_module_mention(map<perm_string,bool>&check_map, PGenerate*s);

/*
 * Convert a string to a time unit or precision.
 *
 * Returns true on failure.
 */
static bool get_ts_const(const char*&cp, int&res, bool is_units)
{
	/* Check for the 1 digit. */
      if (*cp != '1') {
	    if (is_units) {
		  cerr << "Error: Invalid +timescale units constant "
		          "(1st digit)." << endl;
	    } else {
		  cerr << "Error: Invalid +timescale precision constant "
		          "(1st digit)." << endl;
	    }
	    return true;
      }
      cp += 1;

	/* Check the number of zeros after the 1. */
      res = strspn(cp, "0");
      if (res > 2) {
	    if (is_units) {
		  cerr << "Error: Invalid +timescale units constant "
		          "(number of zeros)." << endl;
	    } else {
		  cerr << "Error: Invalid +timescale precision constant "
		          "(number of zeros)." << endl;
	    }
	    return true;
      }
      cp += res;

	/* Now process the scaling string. */
      if (strncmp("s", cp, 1) == 0) {
	    res -= 0;
	    cp += 1;
	    return false;

      } else if (strncmp("ms", cp, 2) == 0) {
	    res -= 3;
	    cp += 2;
	    return false;

      } else if (strncmp("us", cp, 2) == 0) {
	    res -= 6;
	    cp += 2;
	    return false;

      } else if (strncmp("ns", cp, 2) == 0) {
	    res -= 9;
	    cp += 2;
	    return false;

      } else if (strncmp("ps", cp, 2) == 0) {
	    res -= 12;
	    cp += 2;
	    return false;

      } else if (strncmp("fs", cp, 2) == 0) {
	    res -= 15;
	    cp += 2;
	    return false;

      }

      if (is_units) {
	    cerr << "Error: Invalid +timescale units scale." << endl;
      } else {
	    cerr << "Error: Invalid +timescale precision scale." << endl;
      }
      return true;
}

/*
 * Process a string with the following form (no space allowed):
 *
 *   num = < '1' | '10' | '100' >
 *   scale = < 's' | 'ms' | 'us' | 'ns' | 'ps' | 'fs' >
 *
 *   "<num> <scale> '/' <num> <scale>
 *
 * and set the default time units and precision if successful.
 *
 * Return true if we have an error processing the timescale string.
 */
static bool set_default_timescale(const char*ts_string)
{
	/* Because this came from a command file we can not have embedded
	 * space in this string. */
      const char*cp = ts_string;
      int units = 0;
      int prec = 0;

	/* Get the time units. */
      if (get_ts_const(cp, units, true)) return true;

	/* Skip the '/'. */
      if (*cp != '/') {
	    cerr << "Error: +timescale separator '/' is missing." << endl;
	    return true;
      }
      cp += 1;

	/* Get the time precision. */
      if (get_ts_const(cp, prec, false)) return true;

	/* The time unit must be greater than or equal to the precision. */
      if (units < prec) {
	    cerr << "Error: +timescale unit must not be less than the "
	            "precision." << endl;
	    return true;
      }

	/* We have valid units and precision so set the global defaults. */
      def_ts_units = units;
      def_ts_prec = prec;
      return false;
}

/*
 * Read the contents of a config file. This file is a temporary
 * configuration file made by the compiler driver to carry the bulky
 * flags generated from the user. This reduces the size of the command
 * line needed to invoke ivl.
 *
 * Each line of the iconfig file has the format:
 *
 *      <keyword>:<value>
 *
 * The <value> is all the text after the ':' and up to but not
 * including the end of the line. Thus, white spaces and ':'
 * characters may appear here.
 *
 * The valid keys are:
 *
 *    -y:<dir>
 *    -yl:<dir>
 *    -Y:<string>
 *
 *    -T:<min/typ/max>
 *        Select which expression to use.
 *
 *    -t:<target>    (obsolete)
 *        Usually, "-t:dll"
 *
 *    basedir:<path>
 *        Location to look for installed sub-components
 *
 *    debug:<name>
 *        Activate a class of debug messages.
 *
 *    depfile:<path>
 *        Give the path to an output dependency file.
 *
 *    flag:<name>=<string>
 *        Generic compiler flag strings.
 *
 *    functor:<name>
 *        Append a named functor to the processing path.
 *
 *    generation:<1|2|2x|xtypes|no-xtypes|specify|no-specify>
 *        This is the generation flag
 *
 *    ivlpp:<preprocessor command>
 *        This specifies the ivlpp command line used to process
 *        library modules as I read them in.
 *
 *    iwidth:<bits>
 *        This specifies the width of integer variables. (that is,
 *        variables declared using the "integer" keyword.)
 *
 *    library_file:<path>
 *        This marks that a source file with the given path is a
 *        library. Any modules in that file are marked as library
 *        modules.
 *
 *    module:<name>
 *        Load a VPI module.
 *
 *    out:<path>
 *        Path to the output file.
 *
 *    sys_func:<path>
 *        Path to a system functions descriptor table
 *
 *    root:<name>
 *        Specify a root module. There may be multiple of this.
 *
 *    warnings:<string>
 *        Warning flag letters.
 *
 *    ignore_missing_modules:<bool>
 *        true to ignore errors about missing modules
 */
bool had_timescale = false;
static void read_iconfig_file(const char*ipath)
{
      char buf[8*1024];
      vector<pair<char*,bool> > to_build_library_index;

      FILE*ifile = fopen(ipath, "r");
      if (ifile == 0) {
	    cerr << "ERROR: Unable to read config file: " << ipath << endl;
	    return;
      }

      while (fgets(buf, sizeof buf, ifile) != 0) {
	    assert(strlen(buf) < sizeof buf);
	    if ((strlen(buf) == ((sizeof buf) - 1))
		&& buf[sizeof buf -2] != '\n') {
		  cerr << "WARNING: Line buffer overflow reading iconfig file: "
		       << ipath
		       << "." << endl;
		  assert(0);
	    }
	    if (buf[0] == '#')
		  continue;
	    char*cp = strchr(buf, ':');
	    if (cp == 0)
		  continue;

	    *cp++ = 0;
	    char*ep = cp + strlen(cp);
	    while (ep > cp) {
		  ep -= 1;
		  switch (*ep) {
		      case '\r':
		      case '\n':
		      case ' ':
		      case '\t':
			*ep = 0;
			break;
		      default:
			ep = cp;
		  }
	    }

	    if (strcmp(buf, "basedir") == 0) {
		  free((void *)basedir);
		  basedir = strdup(cp);

	    } else if (strcmp(buf, "debug") == 0) {
		  if (strcmp(cp, "scopes") == 0) {
			debug_scopes = true;
			cerr << "debug: Enable scopes debug" << endl;
		  } else if (strcmp(cp,"eval_tree") == 0) {
			debug_eval_tree = true;
			cerr << "debug: Enable eval_tree debug" << endl;
		  } else if (strcmp(cp,"elaborate") == 0) {
			debug_elaborate = true;
			cerr << "debug: Enable elaborate debug" << endl;
		  } else if (strcmp(cp,"emit") == 0) {
			debug_emit = true;
			cerr << "debug: Enable emit debug" << endl;
		  } else if (strcmp(cp,"synth2") == 0) {
			debug_synth2 = true;
			cerr << "debug: Enable synth2 debug" << endl;
		  } else if (strcmp(cp,"optimizer") == 0) {
			debug_optimizer = true;
			cerr << "debug: Enable optimizer debug" << endl;
		  } else {
		  }

	    } else if (strcmp(buf, "depmode") == 0) {
		  depfile_mode = *cp;

	    } else if (strcmp(buf, "depfile") == 0) {
		  depfile_name = strdup(cp);

	    } else if (strcmp(buf, "flag") == 0) {
		  string parm = cp;
		  parm_to_flagmap(parm);

	    } else if (strcmp(buf,"functor") == 0) {
		if (strncmp(cp, "synth", 5) == 0) {
		      synthesis = true;  // We are doing synthesis.
		}
		net_func tmp = name_to_net_func(cp);
		if (tmp == 0) {
		      cerr << "No such design transform function ``"
			   << cp << "''." << endl;
		      flag_errors += 1;
		      break;
		}
		net_func_queue.push(tmp);

	    } else if (strcmp(buf, "generation") == 0) {
		  process_generation_flag(cp);

	    } else if (strcmp(buf, "ivlpp") == 0) {
		  ivlpp_string = strdup(cp);

	    } else if (strcmp(buf, "iwidth") == 0) {
		  integer_width = strtoul(cp,0,10);

	    } else if (strcmp(buf, "widthcap") == 0) {
		  width_cap = strtoul(cp,0,10);

	    } else if (strcmp(buf, "library_file") == 0) {
		  perm_string path = filename_strings.make(cp);
		  library_file_map[path] = true;

	    } else if (strcmp(buf,"module") == 0) {
		  add_vpi_module(cp);

	    } else if (strcmp(buf, "out") == 0) {
		  free((void *)flags["-o"]);
		  flags["-o"] = strdup(cp);

	    } else if (strcmp(buf, "sys_func") == 0) {
		  load_sys_func_table(cp);

	    } else if (strcmp(buf, "root") == 0) {
		  roots.push_back(lex_strings.make(cp));

	    } else if (strcmp(buf,"warnings") == 0) {
		    /* Scan the warnings enable string for warning flags. */
		  for ( ;  *cp ;  cp += 1) switch (*cp) {
		      case 'f':
			warn_floating_nets = true;
			break;
		      case 'i':
			warn_implicit = true;
			break;
		      case 'd':
			warn_implicit_dimensions = true;
			break;
		      case 'l':
			warn_inf_loop = true;
			break;
		      case 's':
			warn_ob_select = true;
			break;
		      case 'p':
			warn_portbinding = true;
			break;
		      case 't':
			warn_timescale = true;
			break;
		      case 'v':
			warn_sens_entire_vec = true;
			break;
		      case 'a':
			warn_sens_entire_arr = true;
			break;
		      case 'n':
			warn_anachronisms = true;
			break;
		      default:
			break;
		  }

	    } else if (strcmp(buf, "ignore_missing_modules") == 0) {
		  if (strcmp(cp, "true") == 0)
		    ignore_missing_modules = true;

	    } else if (strcmp(buf, "-y") == 0) {
		  to_build_library_index.push_back(make_pair(strdup(cp), CASE_SENSITIVE));

	    } else if (strcmp(buf, "-yl") == 0) {
		  to_build_library_index.push_back(make_pair(strdup(cp), false));

	    } else if (strcmp(buf, "-Y") == 0) {
		  library_suff.push_back(strdup(cp));

	    } else if (strcmp(buf,"-t") == 0) {
		    // NO LONGER USED

	    } else if (strcmp(buf,"-T") == 0) {
		  if (strcmp(cp,"min") == 0) {
			min_typ_max_flag = MIN;
			min_typ_max_warn = 0;
		  } else if (strcmp(cp,"typ") == 0) {
			min_typ_max_flag = TYP;
			min_typ_max_warn = 0;
		  } else if (strcmp(cp,"max") == 0) {
			min_typ_max_flag = MAX;
			min_typ_max_warn = 0;
		  } else {
			cerr << "Invalid argument (" << optarg << ") to -T flag."
			     << endl;
			flag_errors += 1;
		  }
	    } else if (strcmp(buf,"defparam") == 0) {
		  parm_to_defparam_list(cp);
	    } else if (strcmp(buf,"timescale") == 0) {
		  if (had_timescale) {
			cerr << "Command File: Warning: default timescale "
			        "is being set multiple times." << endl;
			cerr << "                     : using the last valid "
			        "+timescale found." << endl;
		  }
		  if (set_default_timescale(cp)) {
			cerr << "     : with +timescale+" << cp << "+" << endl;
			flag_errors += 1;
		  } else had_timescale = true;
	    }
      }
      fclose(ifile);
      for (vector<pair<char *, bool> >::iterator it = to_build_library_index.begin() ;
	   it != to_build_library_index.end() ; ++ it ) {
	    build_library_index(it->first, it->second);
	    free(it->first);
      }
}

/*
 * This function reads a list of source file names. Each name starts
 * with the first non-space character, and ends with the last non-space
 * character. Spaces in the middle are OK.
 */
static void read_sources_file(const char*path)
{
      char line_buf[2048];

      FILE*fd = fopen(path, "r");
      if (fd == 0) {
	    cerr << "ERROR: Unable to read source file list: " << path << endl;
	    return;
      }

      while (fgets(line_buf, sizeof line_buf, fd) != 0) {
	      // assertion test that we are not overflowing the line
	      // buffer. Really should make this more robust, but
	      // better to assert then go weird.
	    assert(strlen(line_buf) < sizeof line_buf);
	    if ((strlen(line_buf) == ((sizeof line_buf) - 1))
		&& line_buf[sizeof line_buf -2] != '\n') {
		  cerr << "WARNING: Line buffer overflow reading sources file: "
		       << path
		       << "." << endl;
		  assert(0);
	    }
	    char*cp = line_buf + strspn(line_buf, " \t\r\b\f");
	    char*tail = cp + strlen(cp);
	    while (tail > cp) {
		  if (! isspace((int)tail[-1]))
			break;
		  tail -= 1;
		  tail[0] = 0;
	    }

	    if (cp < tail)
		  source_files.push_back(filename_strings.make(cp));
      }

      fclose(fd);
}

extern Design* elaborate(list <perm_string> root);

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
inline static double cycles_diff(struct tms *, struct tms *) { return 0; }
#endif // ! defined(HAVE_TIMES)

static void EOC_cleanup(void)
{
      cleanup_sys_func_table();

      for (list<const char*>::iterator suf = library_suff.begin() ;
           suf != library_suff.end() ; ++ suf ) {
	    free((void *)*suf);
      }
      library_suff.clear();

      free((void *) basedir);
      free(ivlpp_string);
      free(depfile_name);

      for (map<string, const char*>::iterator flg = flags.begin() ;
           flg != flags.end() ; ++ flg ) {
	    free((void *)flg->second);
      }
      flags.clear();

      lex_strings.cleanup();
      bits_strings.cleanup();
      filename_strings.cleanup();
}

int main(int argc, char*argv[])
{
      bool help_flag = false;
      bool times_flag = false;
      bool version_flag = false;

      const char* net_path = 0;
      const char* pf_path = 0;
      int opt;

      struct tms cycles[5];

#if defined(TRAP_SIGINT_FOR_DEBUG)
      signal(SIGINT, &signals_handler);
#endif
      if( ::getenv("IVL_WAIT_FOR_DEBUGGER") != 0 ) {
          fprintf( stderr, "Waiting for debugger...\n");
          bool debugger_release = false;
          while( !debugger_release )  {
#if defined(__MINGW32__)
              Sleep(1000);
#else
              sleep(1);
#endif
        }
      }
      library_suff.push_back(strdup(".v"));

      flags["-o"] = strdup("a.out");
      min_typ_max_flag = TYP;
      min_typ_max_warn = 10;

      while ((opt = getopt(argc, argv, "C:F:f:hN:P:p:Vv")) != EOF) switch (opt) {

	  case 'C':
	    read_iconfig_file(optarg);
	    break;
	  case 'F':
	    read_sources_file(optarg);
	    break;
	  case 'f':
	    parm_to_flagmap(optarg);
	    break;
	  case 'h':
	    help_flag = true;
	    break;
	  case 'N':
	    net_path = optarg;
	    break;
	  case 'P':
	    pf_path = optarg;
	    break;
	  case 'p':
	    parm_to_flagmap(optarg);
	    break;
	  case 'v':
	    verbose_flag = true;
#          if defined(HAVE_TIMES)
	    times_flag = true;
#          endif
	    flags["VVP_EXTRA_ARGS"] = strdup(" -v");
	    break;
	  case 'V':
	    version_flag = true;
	    break;
	  default:
	    flag_errors += 1;
	    break;
      }

      if (flag_errors)
	    return flag_errors;

      if (version_flag) {
	    cout << "\nIcarus Verilog Parser/Elaborator version "
		 << VERSION << " (" << VERSION_TAG << ")" << endl << endl;
	    cout << COPYRIGHT << endl << endl;
	    cout << NOTICE << endl;

	    cout << " FLAGS DLL " << flags["DLL"] << endl;

	    dll_target_obj.test_version(flags["DLL"]);

	    return 0;
      }

      if (help_flag) {
	    cout << "Icarus Verilog Parser/Elaborator version "
		 << VERSION << " (" << VERSION_TAG << ")"  << endl <<
"usage: ivl <options> <file>\n"
"options:\n"
"\t-C <name>        Config file from driver.\n"
"\t-F <file>        List of source files from driver.\n"
"\t-h               Print usage information, and exit.\n"
"\t-N <file>        Dump the elaborated netlist to <file>.\n"
"\t-P <file>        Write the parsed input to <file>.\n"
"\t-p <assign>      Set a parameter value.\n"
"\t-v               Print progress indications"
#if defined(HAVE_TIMES)
                                           " and execution times"
#endif
                                           ".\n"
"\t-V               Print version and copyright information, and exit.\n"

		  ;
	    return 0;
      }

      int arg = optind;
      while (arg < argc) {
	    perm_string path = filename_strings.make(argv[arg++]);
	    source_files.push_back(path);
      }

      if (source_files.empty()) {
	    cerr << "No input files." << endl;
	    return 1;
      }

      separate_compilation = source_files.size() > 1;

      if( depfile_name ) {
	      depend_file = fopen(depfile_name, "a");
	      if(! depend_file) {
		      perror(depfile_name);
	      }
      }

      lexor_keyword_mask = 0;
      switch (generation_flag) {
        case GN_VER2012:
	  lexor_keyword_mask |= GN_KEYWORDS_1800_2012;
	  // fallthrough
        case GN_VER2009:
	  lexor_keyword_mask |= GN_KEYWORDS_1800_2009;
	  // fallthrough
        case GN_VER2005_SV:
	  lexor_keyword_mask |= GN_KEYWORDS_1800_2005;
	  // fallthrough
        case GN_VER2005:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2005;
	  // fallthrough
        case GN_VER2001:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2001_CONFIG;
	  // fallthrough
        case GN_VER2001_NOCONFIG:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2001;
	  // fallthrough
        case GN_VER1995:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_1995;
      }

      if (gn_cadence_types_flag)
	    lexor_keyword_mask |= GN_KEYWORDS_ICARUS;

      if (gn_verilog_ams_flag)
	    lexor_keyword_mask |= GN_KEYWORDS_VAMS_2_3;

      if (verbose_flag) {
	    if (times_flag)
		  times(cycles+0);

	    cout << "Using language generation: ";
	    switch (generation_flag) {
		case GN_VER1995:
		  cout << "IEEE1364-1995";
		  break;
		case GN_VER2001_NOCONFIG:
		  cout << "IEEE1364-2001-noconfig";
		  break;
		case GN_VER2001:
		  cout << "IEEE1364-2001";
		  break;
		case GN_VER2005:
		  cout << "IEEE1364-2005";
		  break;
		case GN_VER2005_SV:
		  cout << "IEEE1800-2005";
		  break;
		case GN_VER2009:
		  cout << "IEEE1800-2009";
		  break;
		case GN_VER2012:
		  cout << "IEEE1800-2012";
		  break;
	    }

	    if (gn_verilog_ams_flag)
		  cout << ",verilog-ams";

	    if (gn_specify_blocks_flag)
		  cout << ",specify";
	    else
		  cout << ",no-specify";

	    if (gn_interconnect_flag)
		  cout << ",interconnect";
	    else
		  cout << ",no-interconnect";

	    if (gn_cadence_types_flag)
		  cout << ",xtypes";
	    else
		  cout << ",no-xtypes";

	    if (gn_icarus_misc_flag)
		  cout << ",icarus-misc";
	    else
		  cout << ",no-icarus-misc";

	    cout << endl << "PARSING INPUT" << endl;
      }

      const char *flag_tmp = flags["DISABLE_VIRTUAL_PINS"];
      if (flag_tmp) disable_virtual_pins = strcmp(flag_tmp,"true")==0;

      flag_tmp = flags["ARRAY_SIZE_LIMIT"];
      if (flag_tmp) array_size_limit = strtoul(flag_tmp,NULL,0);

      flag_tmp = flags["RECURSIVE_MOD_LIMIT"];
      if (flag_tmp) recursive_mod_limit = strtoul(flag_tmp,NULL,0);

      flag_tmp = flags["DISABLE_CONCATZ_GENERATION"];
      if (flag_tmp) disable_concatz_generation = strcmp(flag_tmp,"true")==0;

	/* Parse the input. Make the pform. */
      int rc = 0;
      for (unsigned idx = 0; idx < source_files.size(); idx += 1) {
	    rc += pform_parse(source_files[idx]);
      }

      pform_finish();

      if (pf_path) {
	    ofstream out (pf_path);
	    out << "PFORM DUMP NATURES:" << endl;
	    for (map<perm_string,ivl_nature_t>::iterator cur = natures.begin()
		       ; cur != natures.end() ; ++ cur ) {
		  pform_dump(out, (*cur).second);
	    }
	    out << "PFORM DUMP DISCIPLINES:" << endl;
	    for (map<perm_string,ivl_discipline_t>::iterator cur = disciplines.begin()
		       ; cur != disciplines.end() ; ++ cur ) {
		  pform_dump(out, (*cur).second);
	    }
	    out << "PFORM DUMP COMPILATION UNITS:" << endl;
	    for (vector<PPackage*>::iterator pac = pform_units.begin()
		       ; pac != pform_units.end() ; ++ pac) {
		  pform_dump(out, *pac);
	    }
	    out << "PFORM DUMP PACKAGES:" << endl;
	    for (vector<PPackage*>::iterator pac = pform_packages.begin()
		       ; pac != pform_packages.end() ; ++ pac) {
		  pform_dump(out, *pac);
	    }
	    out << "PFORM DUMP MODULES:" << endl;
	    for (map<perm_string,Module*>::iterator mod = pform_modules.begin()
		       ; mod != pform_modules.end() ; ++ mod ) {
		  pform_dump(out, (*mod).second);
	    }
	    out << "PFORM DUMP PRIMITIVES:" << endl;
	    for (map<perm_string,PUdp*>::iterator idx = pform_primitives.begin()
		       ; idx != pform_primitives.end() ; ++ idx ) {
		  (*idx).second->dump(out);
	    }
      }

      if (rc) {
	    return rc;
      }

      if (pre_process_fail_count) {
	    cerr << "Preprocessor failed with " << pre_process_fail_count
	         << " errors." << endl;
	    return pre_process_fail_count;
      }


	/* If the user did not give specific module(s) to start with,
	   then look for modules that are not instantiated anywhere.  */

      if (roots.empty()) {
	    map<perm_string,bool> mentioned_p;
	    map<perm_string,Module*>::iterator mod;
	    if (verbose_flag)
		  cout << "LOCATING TOP-LEVEL MODULES" << endl << "  ";
	    for (mod = pform_modules.begin()
		       ; mod != pform_modules.end() ; ++ mod ) {
		  find_module_mention(mentioned_p, mod->second);
	    }

	    for (mod = pform_modules.begin()
		       ; mod != pform_modules.end() ; ++ mod ) {

		  if (!(*mod).second->can_be_toplevel())
			continue;

		    /* Don't choose modules instantiated in other
		       modules. */
		  if (mentioned_p[(*mod).second->mod_name()])
			continue;

		    /* What's left might as well be chosen as a root. */
		  if (verbose_flag)
			cout << " " << (*mod).second->mod_name();
		  roots.push_back((*mod).second->mod_name());
	    }
	    if (verbose_flag)
		  cout << endl;
      }

	/* If there is *still* no guess for the root module, then give
	   up completely, and complain. */

      if (roots.empty()) {
	    cerr << "No top level modules, and no -s option." << endl;
	    return ignore_missing_modules ? 0 : 1;
      }


      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+1);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+1, cycles+0)<<" seconds."<<endl;
	    }
	    cout << "ELABORATING DESIGN" << endl;
      }

	/* Decide if we are going to allow system functions to be called
	 * as tasks. */
      if (gn_system_verilog()) {
	    def_sfunc_as_task = IVL_SFUNC_AS_TASK_WARNING;
      }

	/* On with the process of elaborating the module. */
      Design*des = elaborate(roots);

      if ((des == 0) || (des->errors > 0)) {
	    if (des != 0) {
		  cerr << des->errors
		       << " error(s) during elaboration." << endl;
		  if (net_path) {
			ofstream out (net_path);
			des->dump(out);
		  }
	    } else {
		  cerr << "Elaboration failed" << endl;
	    }

	    goto errors_summary;
      }

      des->set_flags(flags);

      switch(min_typ_max_flag) {
	case MIN:
	    des->set_delay_sel(Design::MIN);
	    break;
	case TYP:
	    des->set_delay_sel(Design::TYP);
	    break;
	case MAX:
	    des->set_delay_sel(Design::MAX);
	    break;
	default:
	    assert(0);
      }

	/* Done with all the pform data. Delete the modules. */
      for (map<perm_string,Module*>::iterator idx = pform_modules.begin()
		 ; idx != pform_modules.end() ; ++ idx ) {

	    delete (*idx).second;
	    (*idx).second = 0;
      }

      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+2);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+2, cycles+1)<<" seconds."<<endl;
	    }
	    cout << "RUNNING FUNCTORS" << endl;
      }

      while (!net_func_queue.empty()) {
	    net_func func = net_func_queue.front();
	    net_func_queue.pop();
	    if (verbose_flag)
		  cerr<<" -F "<<net_func_to_name(func)<< " ..." <<endl;
	    func(des);
      }

      if (verbose_flag) {
	    cout << "CALCULATING ISLANDS" << endl;
      }
      des->join_islands();

      if (net_path) {
	    if (verbose_flag)
		  cerr<<" dumping netlist to " <<net_path<< "..." <<endl;

	    ofstream out (net_path);
	    des->dump(out);
      }

      if (des->errors) {
	    cerr << des->errors
		 << " error(s) in post-elaboration processing." <<
		  endl;
	    return des->errors;
      }

      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+3);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+3, cycles+2)<<" seconds."<<endl;
	    }
      }

      if (verbose_flag) {
	    cout << "CODE GENERATION" << endl;
      }

      if (int emit_rc = des->emit(&dll_target_obj)) {
	    if (emit_rc > 0) {
		  cerr << "error: Code generation had "
		       << emit_rc << " error(s)."
		       << endl;
		  delete des;
		  EOC_cleanup();
		  return 1;
	    } else {
		  cerr << "error: Code generator failure: " << emit_rc << endl;
		  delete des;
		  EOC_cleanup();
		  return -1;
	    }
	    assert(emit_rc);
      }

      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+4);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+4, cycles+3)<<" seconds."<<endl;
	    } else {
		  cout << "DONE." << endl;
	    }
      }

      if (verbose_flag) {
	    cout << "STATISTICS" << endl;
	    cout << "lex_string:"
		 << " add_count=" << lex_strings.add_count()
		 << " hit_count=" << lex_strings.add_hit_count()
		 << endl;
      }

      delete des;
      EOC_cleanup();
      return 0;

 errors_summary:
      if (! missing_modules.empty()) {
	    cerr << "*** These modules were missing:" << endl;

	    map<perm_string,unsigned>::const_iterator idx;
	    for (idx = missing_modules.begin()
		       ; idx != missing_modules.end() ; ++ idx )
		  cerr << "        " << (*idx).first
		       << " referenced " << (*idx).second
		       << " times."<< endl;

	    cerr << "***" << endl;
      }

      int rtn = des? des->errors : 1;
      delete des;
      EOC_cleanup();
      return rtn;
}

static void find_module_mention(map<perm_string,bool>&check_map, Module*mod)
{
      list<PGate*> gates = mod->get_gates();
      list<PGate*>::const_iterator gate;
      for (gate = gates.begin(); gate != gates.end(); ++ gate ) {
	    PGModule*tmp = dynamic_cast<PGModule*>(*gate);
	    if (tmp) {
		    // Note that this module has been instantiated
		  check_map[tmp->get_type()] = true;
	    }
      }

      list<PGenerate*>::const_iterator cur;
      for (cur = mod->generate_schemes.begin()
		 ; cur != mod->generate_schemes.end() ; ++ cur ) {
	    find_module_mention(check_map, *cur);
      }
}

static void find_module_mention(map<perm_string,bool>&check_map, PGenerate*schm)
{
      list<PGate*>::const_iterator gate;
      for (gate = schm->gates.begin(); gate != schm->gates.end(); ++ gate ) {
	    PGModule*tmp = dynamic_cast<PGModule*>(*gate);
	    if (tmp) {
		    // Note that this module has been instantiated
		  check_map[tmp->get_type()] = true;
	    }
      }

      list<PGenerate*>::const_iterator cur;
      for (cur = schm->generate_schemes.begin()
		 ; cur != schm->generate_schemes.end() ; ++ cur ) {
	    find_module_mention(check_map, *cur);
      }
}

void pre_process_failed(const char*text)
{
      const char*num_start = strchr(text, '(') + 1;
      unsigned long res;
      char*rem;
      res = strtoul(num_start, &rem, 10);
      assert(res > 0);
      assert(rem[0] == ')');
      pre_process_fail_count += res;
}
