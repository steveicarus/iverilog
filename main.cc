
const char COPYRIGHT[] =
          "Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)";

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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: main.cc,v 1.45 2001/07/16 18:14:56 steve Exp $"
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

# include  <stdio.h>
# include  <iostream.h>
# include  <fstream>
# include  <queue>
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
# include  "netlist.h"
# include  "target.h"
# include  "compiler.h"

#if defined(__MINGW32__) && !defined(HAVE_GETOPT_H)
extern "C" int getopt(int argc, char*argv[], const char*fmt);
extern "C" int optind;
extern "C" const char*optarg;
#endif

const char VERSION[] = "$Name:  $ $State: Exp $";

const char*target = "null";
string start_module = "";

map<string,string> flags;

/*
 * These are the warning enable flags.
 */
bool warn_implicit = false;


static void parm_to_flagmap(const string&flag)
{
      string key, value;
      unsigned off = flag.find('=');
      if (off > flag.size()) {
	    key = flag;
	    value = "";

      } else {
	    key = flag.substr(0, off);
	    value = flag.substr(off+1);
      }

      flags[key] = value;
}


extern Design* elaborate(const map<string,Module*>&modules,
			 const map<string,PUdp*>&primitives,
			 const string&root);

extern void cprop(Design*des);
extern void synth(Design*des);
extern void syn_rules(Design*des);
extern void nodangle(Design*des);
extern void xnfio(Design*des);

typedef void (*net_func)(Design*);
static struct net_func_map {
      const char*name;
      void (*func)(Design*);
} func_table[] = {
      { "cprop",   &cprop },
      { "nodangle",&nodangle },
      { "synth",   &synth },
      { "syn-rules",   &syn_rules },
      { "xnfio",   &xnfio },
      { 0, 0 }
};

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
      bool verbose_flag = false;
      bool times_flag = false;

      const char* net_path = 0;
      const char* pf_path = 0;
      const char* warn_en = "";
      int opt;
      unsigned flag_errors = 0;
      queue<net_func> net_func_queue;

      struct tms cycles[5];

      flags["VPI_MODULE_LIST"] = "system";
      flags["-o"] = "a.out";
      min_typ_max_flag = TYP;
      min_typ_max_warn = 10;

      while ((opt = getopt(argc, argv, "F:f:hm:N:o:P:p:s:T:t:VvW:")) != EOF) switch (opt) {
	  case 'F': {
		net_func tmp = name_to_net_func(optarg);
		if (tmp == 0) {
		      cerr << "No such design transform function ``"
			   << optarg << "''." << endl;
		      flag_errors += 1;
		      break;
		}
		net_func_queue.push(tmp);
		break;
	  }
	  case 'f':
	    parm_to_flagmap(optarg);
	    break;
	  case 'h':
	    help_flag = true;
	    break;
	  case 'm':
	    flags["VPI_MODULE_LIST"] = flags["VPI_MODULE_LIST"]+","+optarg;
	    break;
	  case 'N':
	    net_path = optarg;
	    break;
	  case 'o':
	    flags["-o"] = optarg;
	    break;
	  case 'P':
	    pf_path = optarg;
	    break;
	  case 'p':
	    parm_to_flagmap(optarg);
	    break;
	  case 's':
	    start_module = optarg;
	    break;
	  case 'T':
	    if (strcmp(optarg,"min") == 0) {
		  min_typ_max_flag = MIN;
		  min_typ_max_warn = 0;
	    } else if (strcmp(optarg,"typ") == 0) {
		  min_typ_max_flag = TYP;
		  min_typ_max_warn = 0;
	    } else if (strcmp(optarg,"max") == 0) {
		  min_typ_max_flag = MAX;
		  min_typ_max_warn = 0;
	    } else {
		  cerr << "Invalid argument (" << optarg << ") to -T flag."
		       << endl;
		  flag_errors += 1;
	    }
	    break;
	  case 't':
	    target = optarg;
	    break;
	  case 'v':
	    verbose_flag = true;
#          if defined(HAVE_TIMES)
	    times_flag = true;
#          endif
	    break;
	  case 'V':
	    cout << "Icarus Verilog version " << VERSION << endl;
	    cout << COPYRIGHT << endl;
	    cout << endl << NOTICE << endl;
	    return 0;
	  case 'W':
	    warn_en = optarg;
	    break;
	  default:
	    flag_errors += 1;
	    break;
      }

      if (flag_errors)
	    return flag_errors;

      if (help_flag) {
	    cout << "Icarus Verilog version " << VERSION << endl <<
"usage: ivl <options> <file>\n"
"options:\n"
"\t-F <name>        Apply netlist function <name>.\n"
"\t-h               Print usage information, and exit.\n"
"\t-m <module>      Load vpi module <module>.\n"
"\t-N <file>        Dump the elaborated netlist to <file>.\n"
"\t-o <file>        Write output to <file>.\n"
"\t-P <file>        Write the parsed input to <file>.\n"
"\t-p <assign>      Set a parameter value.\n"
"\t-s <module>      Select the top-level module.\n"
"\t-T [min|typ|max] Select timing corner.\n"
"\t-t <name>        Select target <name>.\n"
"\t-v               Print progress indications"
#if defined(HAVE_TIMES)
                                           " and execution times"
#endif
                                           ".\n"
"\t-V               Print version and copyright information, and exit.\n"

		  ;
	    cout << "Netlist functions:" << endl;
	    for (unsigned idx = 0 ;  func_table[idx].name ;  idx += 1)
		  cout << "\t-F " << func_table[idx].name << endl;
	    cout << "Target types:" << endl;
	    for (unsigned idx = 0 ;  target_table[idx] ;  idx += 1)
		  cout << "\t-t " << target_table[idx]->name << endl;
	    return 0;
      }

      if (optind == argc) {
	    cerr << "No input files." << endl;
	    return 1;
      }

	/* Scan the warnings enable string for warning flags. */
      for (const char*cp = warn_en ;  *cp ;  cp += 1) switch (*cp) {
	  case 'i':
	    warn_implicit = true;
	    break;
	  default:
	    break;
      }

      if (verbose_flag) {
	    if (times_flag)
		  times(cycles+0);
	    cout << "PARSING INPUT ..." << endl;
      }

	/* Parse the input. Make the pform. */
      map<string,Module*> modules;
      map<string,PUdp*>   primitives;
      int rc = pform_parse(argv[optind], modules, primitives);

      if (pf_path) {
	    ofstream out (pf_path);
	    out << "PFORM DUMP MODULES:" << endl;
	    for (map<string,Module*>::iterator mod = modules.begin()
		       ; mod != modules.end()
		       ; mod ++ ) {
		  pform_dump(out, (*mod).second);
	    }
	    out << "PFORM DUMP PRIMITIVES:" << endl;
	    for (map<string,PUdp*>::iterator idx = primitives.begin()
		       ; idx != primitives.end()
		       ; idx ++ ) {
		  (*idx).second->dump(out);
	    }
      }

      if (rc) {
	    return rc;
      }


	/* If the user did not give a specific module to start with,
	   then look for the single module that has no ports. If there
	   are multiple modules with no ports, then give up. */

      if (start_module == "") {
	    for (map<string,Module*>::iterator mod = modules.begin()
		       ; mod != modules.end()
		       ; mod ++ ) {
		  Module*cur = (*mod).second;
		  if (cur->port_count() == 0)
			if (start_module == "") {
			      start_module = cur->get_name();
		        } else {
			      cerr << "More then 1 top level module."
				   << endl;
			      return 1;
			}
	    }
      }

	/* If the previous attempt to find a start module failed, but
	   there is only one module, then just let the user get away
	   with it. */

      if ((start_module == "") && (modules.size() == 1)) {
	    map<string,Module*>::iterator mod = modules.begin();
	    Module*cur = (*mod).second;
	    start_module = cur->get_name();
      }

	/* If there is *still* no guess for the root module, then give
	   up completely, and complain. */

      if (start_module == "") {
	    cerr << "No top level modules, and no -s option." << endl;
	    return 1;
      }


      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+1);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+1, cycles+0)<<" seconds."<<endl;
	    }
	    cout << "ELABORATING DESIGN -s "<<start_module<<" ..." << endl;
      }

	/* On with the process of elaborating the module. */
      Design*des = elaborate(modules, primitives, start_module);
      if (des == 0) {
	    cerr << start_module << ": error: "
		 << "Unable to elaborate module." << endl;
	    return 1;
      }

      if (des->errors) {
	    cerr << start_module << ": error: " << des->errors
		 << " elaborating module." << endl;
	    return des->errors;
      }

      des->set_flags(flags);


      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+2);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+2, cycles+1)<<" seconds."<<endl;
	    }
	    cout << "RUNNING FUNCTORS ..." << endl;
      }

      while (!net_func_queue.empty()) {
	    net_func func = net_func_queue.front();
	    net_func_queue.pop();
	    if (verbose_flag)
		  cerr<<" -F "<<net_func_to_name(func)<<endl;
	    func(des);
      }

      if (net_path) {
	    ofstream out (net_path);
	    des->dump(out);
      }


      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+3);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+3, cycles+2)<<" seconds."<<endl;
	    }
	    cout << "CODE GENERATION -t "<<target<<" ..." << endl;
      }

      bool emit_rc = emit(des, target);
      if (!emit_rc) {
	    cerr << "error: Code generation had errors." << endl;
	    return 1;
      }

      if (verbose_flag) {
	    if (times_flag) {
		  times(cycles+4);
		  cerr<<" ... done, "
		      <<cycles_diff(cycles+4, cycles+3)<<" seconds."<<endl;
	    } else
		  cout << "DONE." << endl;
      }

      return 0;
}

/*
 * $Log: main.cc,v $
 * Revision 1.45  2001/07/16 18:14:56  steve
 *  Reshuffle -v and -V flags of ivl. (Stephan Boettcher)
 *
 * Revision 1.44  2001/07/03 04:09:24  steve
 *  Generate verbuse status messages (Stephan Boettcher)
 *
 * Revision 1.43  2001/07/02 01:57:27  steve
 *  Add the -V flag, and some verbose messages.
 *
 * Revision 1.42  2001/06/23 18:41:02  steve
 *  Include stdlib.h
 *
 * Revision 1.41  2001/05/20 17:35:05  steve
 *  declare getopt by hand in mingw32 compile.
 *
 * Revision 1.40  2001/01/20 19:02:05  steve
 *  Switch hte -f flag to the -p flag.
 *
 * Revision 1.39  2000/11/22 20:48:32  steve
 *  Allow sole module to be a root.
 *
 * Revision 1.38  2000/09/12 01:17:40  steve
 *  Version information for vlog_vpi_info.
 *
 * Revision 1.37  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.36  2000/07/29 17:58:21  steve
 *  Introduce min:typ:max support.
 *
 * Revision 1.35  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.34  2000/05/13 20:55:47  steve
 *  Use yacc based synthesizer.
 *
 * Revision 1.33  2000/05/08 05:29:43  steve
 *  no need for nobufz functor.
 *
 * Revision 1.32  2000/05/03 22:14:31  steve
 *  More features of ivl available through iverilog.
 *
 * Revision 1.31  2000/04/12 20:02:53  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 */

