/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: main.cc,v 1.17 1999/06/17 05:33:12 steve Exp $"
#endif

# include  <stdio.h>
# include  <iostream.h>
# include  <fstream>
# include  <queue>
# include  <map>
# include  <unistd.h>
# include  "pform.h"
# include  "netlist.h"
# include  "target.h"


const char*target = "null";
string start_module = "";

map<string,string> flags;

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
extern void emit(ostream&o, const Design*, const char*);

extern void cprop(Design*des);
extern void propinit(Design*des);
extern void sigfold(Design*des);
extern void stupid(Design*des);
extern void nobufz(Design*des);
extern void xnfio(Design*des);

typedef void (*net_func)(Design*);
static struct net_func_map {
      const char*name;
      void (*func)(Design*);
} func_table[] = {
      { "cprop",   &cprop },
      { "nobufz",  &nobufz },
      { "propinit", &propinit },
      { "sigfold", &sigfold },
      { "stupid",  &stupid },
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


int main(int argc, char*argv[])
{
      bool help_flag = false;
      const char* net_path = 0;
      const char* out_path = 0;
      const char* pf_path = 0;
      int opt;
      unsigned flag_errors = 0;
      queue<net_func> net_func_queue;

      while ((opt = getopt(argc, argv, "F:f:hN:o:P:s:t:")) != EOF) switch (opt) {
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
	  case 'N':
	    net_path = optarg;
	    break;
	  case 'o':
	    out_path = optarg;
	    break;
	  case 'P':
	    pf_path = optarg;
	    break;
	  case 's':
	    start_module = optarg;
	    break;
	  case 't':
	    target = optarg;
	    break;
	  default:
	    flag_errors += 1;
	    break;
      }

      if (flag_errors)
	    return flag_errors;

      if (help_flag) {
	    cout << "Netlist functions:" << endl;
	    for (unsigned idx = 0 ;  func_table[idx].name ;  idx += 1)
		  cout << "    " << func_table[idx].name << endl;
	    cout << "Target types:" << endl;
	    for (unsigned idx = 0 ;  target_table[idx] ;  idx += 1)
		  cout << "    " << target_table[idx]->name << endl;
	    return 0;
      }

      if (optind == argc) {
	    cerr << "No input files." << endl;
	    return 1;
      }

	/* Parse the input. Make the pform. */
      map<string,Module*> modules;
      map<string,PUdp*>   primitives;
      int rc = pform_parse(argv[optind], modules, primitives);

      if (rc) {
	    return rc;
      }

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

      if (start_module == "") {
	    for (map<string,Module*>::iterator mod = modules.begin()
		       ; mod != modules.end()
		       ; mod ++ ) {
		  Module*cur = (*mod).second;
		  if (cur->ports.count() == 0)
			if (start_module == "") {
			      start_module = cur->get_name();
		        } else {
			      cerr << "More then 1 top level module."
				   << endl;
			      return 1;
			}
	    }
      }

	/* Select a root module, and elaborate the design. */
      if (start_module == "") {
	    cerr << "No top level modules, and no -s option." << endl;
	    return 1;
      }

      Design*des = elaborate(modules, primitives, start_module);
      if (des == 0) {
	    cerr << "Unable to elaborate module " << start_module <<
		  "." << endl;
	    return 1;
      }
      if (des->errors) {
	    cerr << des->errors << " error(s) elaborating design." << endl;
	    return des->errors;
      }

      des->set_flags(flags);


      while (!net_func_queue.empty()) {
	    net_func func = net_func_queue.front();
	    net_func_queue.pop();
	    func(des);
      }

      if (net_path) {
	    ofstream out (net_path);
	    des->dump(out);
      }


      if (out_path) {
	    ofstream out;
	    out.open(out_path);
	    if (! out.is_open()) {
		  cerr << "Unable to open " << out_path << " for writing."
		       << endl;
		  return 1;
	    }

	    emit(out, des, target);

      } else {
	    emit(cout, des, target);
      }

      return 0;
}

/*
 * $Log: main.cc,v $
 * Revision 1.17  1999/06/17 05:33:12  steve
 *  Redundant declaration of pform_parse.
 *
 * Revision 1.16  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.15  1999/05/05 03:27:15  steve
 *  More intelligent selection of module to elaborate.
 *
 * Revision 1.14  1999/04/23 04:34:32  steve
 *  Make debug output file parameters.
 *
 * Revision 1.13  1999/02/01 00:26:49  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.12  1999/01/24 01:35:36  steve
 *  Support null target for generating no output.
 *
 * Revision 1.11  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.10  1998/12/09 04:02:47  steve
 *  Support the include directive.
 *
 * Revision 1.9  1998/12/07 04:53:17  steve
 *  Generate OBUF or IBUF attributes (and the gates
 *  to garry them) where a wire is a pad. This involved
 *  figuring out enough of the netlist to know when such
 *  was needed, and to generate new gates and signales
 *  to handle what's missing.
 *
 * Revision 1.8  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.7  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.6  1998/11/25 02:35:53  steve
 *  Parse UDP primitives all the way to pform.
 *
 * Revision 1.5  1998/11/18 04:25:22  steve
 *  Add -f flags for generic flag key/values.
 *
 * Revision 1.4  1998/11/16 05:03:52  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 * Revision 1.3  1998/11/13 06:23:17  steve
 *  Introduce netlist optimizations with the
 *  cprop function to do constant propogation.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:28:59  steve
 *  Introduce verilog to CVS.
 *
 */

