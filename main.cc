
const char COPYRIGHT[] =
          "Copyright (c) 1998-2008 Stephen Williams (steve@icarus.com)";

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
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;

# include  <stdio.h>
# include  <iostream>
# include  <fstream>
# include  <queue>
# include  <cstring>
# include  <list>
# include  <map>
# include  <unistd.h>
# include  <stdlib.h>
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

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

#if defined(__CYGWIN32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

/* Count errors detected in flag processing. */
unsigned flag_errors = 0;

const char*basedir = ".";

/*
 * These are the language support control flags. These support which
 * language features (the generation) to support. The generation_flag
 * is a major mode, and the gn_* flags control specific sub-features.
 */
generation_t generation_flag = GN_DEFAULT;
bool gn_icarus_misc_flag = true;
bool gn_cadence_types_flag = true;
bool gn_specify_blocks_flag = true;
bool gn_io_range_error_flag = true;
bool gn_verilog_ams_flag = false;

map<string,const char*> flags;
char*vpi_module_list = 0;

map<perm_string,unsigned> missing_modules;
map<perm_string,bool> library_file_map;

list<const char*> library_suff;

list<perm_string> roots;

char*ivlpp_string = 0;

char* depfile_name = NULL;
FILE *depend_file = NULL;

/*
 * These are the warning enable flags.
 */
bool warn_implicit  = false;
bool warn_timescale = false;
bool warn_portbinding = false;
bool warn_inf_loop = false;

bool error_implicit = false;

/*
 * Debug message class flags.
 */
bool debug_scopes = false;
bool debug_eval_tree = false;
bool debug_elaborate = false;
bool debug_elab_pexpr = false;
bool debug_synth2 = false;
bool debug_optimizer = false;
bool debug_automatic = false;

/*
 * Verbose messages enabled.
 */
bool verbose_flag = false;

unsigned integer_width = 32;

/*
 * Keep a heap of identifier strings that I encounter. This is a more
 * efficient way to allocate those strings.
 */
StringHeapLex lex_strings;

StringHeapLex filename_strings;

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
extern void synth(Design*des);
extern void synth2(Design*des);
extern void syn_rules(Design*des);
extern void nodangle(Design*des);

typedef void (*net_func)(Design*);
static struct net_func_map {
      const char*name;
      void (*func)(Design*);
} func_table[] = {
      { "cprop",   &cprop },
      { "nodangle",&nodangle },
      { "synth",   &synth },
      { "synth2",  &synth2 },
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

      } else if (strcmp(gen,"2005") == 0) {
	    generation_flag = GN_VER2005;

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

      } else if (strcmp(gen,"verilog-ams") == 0) {
	    gn_verilog_ams_flag = true;

      } else if (strcmp(gen,"no-verilog-ams") == 0) {
	    gn_verilog_ams_flag = false;

      } else if (strcmp(gen,"io-range-error") == 0) {
	    gn_io_range_error_flag = true;

      } else if (strcmp(gen,"no-io-range-error") == 0) {
	    gn_io_range_error_flag = false;

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
	    value = "";

      } else {
	    key = flag.substr(0, off);
	    value = strdup(flag.substr(off+1).c_str());
      }

      flags[key] = value;
}

static void find_module_mention(map<perm_string,bool>&check_map, Module*m);
static void find_module_mention(map<perm_string,bool>&check_map, PGenerate*s);

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
 */
static void read_iconfig_file(const char*ipath)
{
      char buf[8*1024];

      FILE*ifile = fopen(ipath, "r");
      if (ifile == 0) {
	    cerr << "ERROR: Unable to read config file: " << ipath << endl;
	    return;
      }

      while (fgets(buf, sizeof buf, ifile) != 0) {
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
		  } else if (strcmp(cp,"elab_pexpr") == 0) {
			debug_elab_pexpr = true;
			cerr << "debug: Enable elaborate-pexpr debug" << endl;
		  } else if (strcmp(cp,"synth2") == 0) {
			debug_synth2 = true;
			cerr << "debug: Enable synth2 debug" << endl;
		  } else if (strcmp(cp,"optimizer") == 0) {
			debug_optimizer = true;
			cerr << "debug: Enable optimizer debug" << endl;
		  } else if (strcmp(cp,"automatic") == 0) {
			debug_automatic = true;
		  } else {
		  }

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

	    } else if (strcmp(buf, "library_file") == 0) {
		  perm_string path = filename_strings.make(cp);
		  library_file_map[path] = true;

	    } else if (strcmp(buf,"module") == 0) {
		  if (vpi_module_list == 0) {
			vpi_module_list = strdup(cp);

		  } else {
			char*tmp = (char*)realloc(vpi_module_list,
						  strlen(vpi_module_list)
						  + strlen(cp)
						  + 2);
			strcat(tmp, ",");
			strcat(tmp, cp);
			vpi_module_list = tmp;
		  }
		  flags["VPI_MODULE_LIST"] = vpi_module_list;

	    } else if (strcmp(buf, "out") == 0) {
		  flags["-o"] = strdup(cp);

	    } else if (strcmp(buf, "sys_func") == 0) {
		  load_sys_func_table(cp);

	    } else if (strcmp(buf, "root") == 0) {
		  roots.push_back(lex_strings.make(cp));

	    } else if (strcmp(buf,"warnings") == 0) {
		    /* Scan the warnings enable string for warning flags. */
		  for ( ;  *cp ;  cp += 1) switch (*cp) {
		      case 'i':
			warn_implicit = true;
			break;
		      case 'l':
			warn_inf_loop = true;
			break;
		      case 'p':
			warn_portbinding = true;
			break;
		      case 't':
			warn_timescale = true;
			break;
		      default:
			break;
		  }

	    } else if (strcmp(buf, "-y") == 0) {
		  build_library_index(cp, CASE_SENSITIVE);

	    } else if (strcmp(buf, "-yl") == 0) {
		  build_library_index(cp, false);

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

	    }
      }
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
inline static double cycles_diff(struct tms *a, struct tms *b) { return 0; }
#endif // ! defined(HAVE_TIMES)

int main(int argc, char*argv[])
{
      bool help_flag = false;
      bool times_flag = false;
      bool version_flag = false;

      const char* net_path = 0;
      const char* pf_path = 0;
      int opt;

      struct tms cycles[5];

      library_suff.push_back(".v");

      vpi_module_list = strdup("system");
      flags["VPI_MODULE_LIST"] = vpi_module_list;
      flags["-o"] = "a.out";
      min_typ_max_flag = TYP;
      min_typ_max_warn = 10;

      while ((opt = getopt(argc, argv, "C:f:hN:P:p:Vv")) != EOF) switch (opt) {

	  case 'C':
	    read_iconfig_file(optarg);
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

	    dll_target_obj.test_version(flags["DLL"]);

	    return 0;
      }

      if (help_flag) {
	    cout << "Icarus Verilog Parser/Elaborator version "
		 << VERSION << " (" << VERSION_TAG << ")"  << endl <<
"usage: ivl <options> <file>\n"
"options:\n"
"\t-C <name>        Config file from driver.\n"
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

      if (optind == argc) {
	    cerr << "No input files." << endl;
	    return 1;
      }

      if( depfile_name ) {
	      depend_file = fopen(depfile_name, "a");
	      if(! depend_file) {
		      perror(depfile_name);
	      }
      }

      lexor_keyword_mask = 0;
      switch (generation_flag) {
        case GN_VER1995:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_1995;
	  break;
        case GN_VER2001:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_1995;
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2001;
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2001_CONFIG;
	  break;
        case GN_VER2005:
	  lexor_keyword_mask |= GN_KEYWORDS_1364_1995;
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2001;
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2001_CONFIG;
	  lexor_keyword_mask |= GN_KEYWORDS_1364_2005;
	  break;
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
		case GN_VER2001:
		  cout << "IEEE1364-2001";
		  break;
		case GN_VER2005:
		  cout << "IEEE1364-2005";
		  break;
	    }

	    if (gn_verilog_ams_flag)
		  cout << ",verilog-ams";

	    if (gn_specify_blocks_flag)
		  cout << ",specify";
	    else
		  cout << ",no-specify";

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

	/* Parse the input. Make the pform. */
      int rc = pform_parse(argv[optind]);

      if (pf_path) {
	    ofstream out (pf_path);
	    out << "PFORM DUMP NATURES:" << endl;
	    for (map<perm_string,ivl_nature_t>::iterator cur = natures.begin()
		       ; cur != natures.end() ; cur ++ ) {
		  pform_dump(out, (*cur).second);
	    }
	    out << "PFORM DUMP DISCIPLINES:" << endl;
	    for (map<perm_string,ivl_discipline_t>::iterator cur = disciplines.begin()
		       ; cur != disciplines.end() ; cur ++ ) {
		  pform_dump(out, (*cur).second);
	    }
	    out << "PFORM DUMP MODULES:" << endl;
	    for (map<perm_string,Module*>::iterator mod = pform_modules.begin()
		       ; mod != pform_modules.end()
		       ; mod ++ ) {
		  pform_dump(out, (*mod).second);
	    }
	    out << "PFORM DUMP PRIMITIVES:" << endl;
	    for (map<perm_string,PUdp*>::iterator idx = pform_primitives.begin()
		       ; idx != pform_primitives.end()
		       ; idx ++ ) {
		  (*idx).second->dump(out);
	    }
      }

      if (rc) {
	    return rc;
      }


	/* If the user did not give specific module(s) to start with,
	   then look for modules that are not instantiated anywhere.  */

      if (roots.empty()) {
	    map<perm_string,bool> mentioned_p;
	    map<perm_string,Module*>::iterator mod;
	    if (verbose_flag)
		  cout << "LOCATING TOP-LEVEL MODULES" << endl << "  ";
	    for (mod = pform_modules.begin()
		       ; mod != pform_modules.end()
		       ; mod++) {
		  find_module_mention(mentioned_p, mod->second);
	    }

	    for (mod = pform_modules.begin()
		       ; mod != pform_modules.end()
		       ; mod++) {

		    /* Don't choose library modules. */
		  if ((*mod).second->library_flag)
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
	    return 1;
      }


      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+1);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+1, cycles+0)<<" seconds."<<endl;
	    }
	    cout << "ELABORATING DESIGN" << endl;
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

	/* Done with all the pform data. Delete the modules. */
      for (map<perm_string,Module*>::iterator idx = pform_modules.begin()
		 ; idx != pform_modules.end() ; idx ++) {

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
		       << emit_rc << " errors."
		       << endl;
		  return 1;
	    }
	    if (emit_rc < 0) {
		  cerr << "error: Code generator failure: " << emit_rc << endl;
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

      return 0;

 errors_summary:
      if (missing_modules.size() > 0) {
	    cerr << "*** These modules were missing:" << endl;

	    map<perm_string,unsigned>::const_iterator idx;
	    for (idx = missing_modules.begin()
		       ; idx != missing_modules.end()
		       ; idx ++)
		  cerr << "        " << (*idx).first
		       << " referenced " << (*idx).second
		       << " times."<< endl;

	    cerr << "***" << endl;
      }

      return des? des->errors : 1;
}

static void find_module_mention(map<perm_string,bool>&check_map, Module*mod)
{
      list<PGate*> gates = mod->get_gates();
      list<PGate*>::const_iterator gate;
      for (gate = gates.begin(); gate != gates.end(); gate++) {
	    PGModule*tmp = dynamic_cast<PGModule*>(*gate);
	    if (tmp) {
		    // Note that this module has been instantiated
		  check_map[tmp->get_type()] = true;
	    }
      }

      list<PGenerate*>::const_iterator cur;
      for (cur = mod->generate_schemes.begin()
		 ; cur != mod->generate_schemes.end() ; cur ++) {
	    find_module_mention(check_map, *cur);
      }
}

static void find_module_mention(map<perm_string,bool>&check_map, PGenerate*schm)
{
      list<PGate*>::const_iterator gate;
      for (gate = schm->gates.begin(); gate != schm->gates.end(); gate++) {
	    PGModule*tmp = dynamic_cast<PGModule*>(*gate);
	    if (tmp) {
		    // Note that this module has been instantiated
		  check_map[tmp->get_type()] = true;
	    }
      }

      list<PGenerate*>::const_iterator cur;
      for (cur = schm->generate_schemes.begin()
		 ; cur != schm->generate_schemes.end() ; cur ++) {
	    find_module_mention(check_map, *cur);
      }
}
