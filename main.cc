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
#ident "$Id: main.cc,v 1.2 1998/11/07 17:05:05 steve Exp $"
#endif

# include  <stdio.h>
# include  <iostream.h>
# include  <fstream>
# include  <unistd.h>
# include  "pform.h"
# include  "netlist.h"
# include  "target.h"

extern void pform_parse();

const char*vl_file = "";
const char*target = "verilog";
string start_module = "";

extern Design* elaborate(const list<Module*>&modules, const string&root);
extern void emit(ostream&o, const Design*, const char*);
extern void stupid(Design*des);

int main(int argc, char*argv[])
{
      bool dump_flag = false;
      bool optimize_flag = false;
      const char* out_path = 0;
      int opt;
      unsigned flag_errors = 0;

      while ((opt = getopt(argc, argv, "DOo:s:t:")) != EOF) switch (opt) {
	  case 'D':
	    dump_flag = true;
	    break;
	  case 'O':
	    optimize_flag = true;
	    break;
	  case 'o':
	    out_path = optarg;
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

      if (optind == argc) {
	    cerr << "No input files." << endl;
	    return 1;
      }

	/* Open the input (source) file. */
      vl_file = argv[optind];
      FILE*input = fopen(vl_file, "r");
      if (input == 0) {
	    cerr << "Unable to open " <<vl_file << "." << endl;
	    return 1;
      }

	/* Parse the input. Make the pform. */
      list<Module*>modules;
      int rc = pform_parse(input, modules);

      if (rc) {
	    return rc;
      }

      if (dump_flag) {
	    ofstream out ("a.pf");
	    out << "PFORM DUMP:" << endl;
	    for (list<Module*>::iterator mod = modules.begin()
		       ; mod != modules.end()
		       ; mod ++ ) {
		  pform_dump(out, *mod);
	    }
      }


	/* Select a root module, and elaborate the design. */
      if ((start_module == "") && (modules.size() == 1)) {
	    Module*mod = modules.front();
	    start_module = mod->get_name();
      }

      Design*des = elaborate(modules, start_module);
      if (des == 0) {
	    cerr << "Unable to elaborate design." << endl;
	    return 1;
      }

      if (optimize_flag) {
	    stupid(des);
      }

      if (dump_flag) {
	    ofstream out ("a.net");
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

