
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
#ident "$Id: main.cc,v 1.29 2000/02/23 02:56:54 steve Exp $"
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
#if defined(HAVE_GETOPT_H)
# include  <getopt.h>
#endif
# include  "pform.h"
# include  "netlist.h"
# include  "target.h"

const char VERSION[] = "$Name:  $ $State: Exp $";

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

extern void cprop(Design*des);
extern void propinit(Design*des);
extern void synth(Design*des);
extern void nobufz(Design*des);
extern void nodangle(Design*des);
extern void xnfio(Design*des);
extern void xnfsyn(Design*des);

typedef void (*net_func)(Design*);
static struct net_func_map {
      const char*name;
      void (*func)(Design*);
} func_table[] = {
      { "cprop",   &cprop },
      { "nobufz",  &nobufz },
      { "nodangle",&nodangle },
      { "propinit",&propinit },
      { "synth",   &synth },
      { "xnfio",   &xnfio },
      { "xnfsyn",  &xnfsyn },
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

      flags["VPI_MODULE_LIST"] = "system";

      while ((opt = getopt(argc, argv, "F:f:hm:N:o:P:s:t:v")) != EOF) switch (opt) {
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
	  case 'v':
	    cout << "Icarus Verilog version " << VERSION << endl;
	    cout << COPYRIGHT << endl;
	    cout << endl << NOTICE << endl;
	    return 0;
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


      bool emit_rc;
      if (out_path) {
	    ofstream out;
	    out.open(out_path);
	    if (! out.is_open()) {
		  cerr << "Unable to open " << out_path << " for writing."
		       << endl;
		  return 1;
	    }

	    emit_rc = emit(out, des, target);

      } else {
	    emit_rc = emit(cout, des, target);
      }

      if (!emit_rc) {
	    cerr << "internal error: Code generation had errors." << endl;
	    return 1;
      }

      return 0;
}

/*
 * $Log: main.cc,v $
 * Revision 1.29  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.28  2000/01/13 05:11:25  steve
 *  Support for multiple VPI modules.
 *
 * Revision 1.27  1999/12/30 17:37:14  steve
 *  Remove the now useless sigfold functor.
 *
 * Revision 1.26  1999/11/29 17:02:21  steve
 *  include getopt if present.
 *
 * Revision 1.25  1999/11/18 03:52:19  steve
 *  Turn NetTmp objects into normal local NetNet objects,
 *  and add the nodangle functor to clean up the local
 *  symbols generated by elaboration and other steps.
 *
 * Revision 1.24  1999/11/01 02:07:40  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.23  1999/09/22 16:57:23  steve
 *  Catch parallel blocks in vvm emit.
 *
 * Revision 1.22  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 *
 * Revision 1.21  1999/07/18 05:52:46  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 * Revision 1.20  1999/07/17 22:01:13  steve
 *  Add the functor interface for functor transforms.
 *
 * Revision 1.19  1999/07/10 23:29:21  steve
 *  pform even on parse errors.
 *
 * Revision 1.18  1999/06/19 03:46:42  steve
 *  Add the -v switch.
 *
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
 */

