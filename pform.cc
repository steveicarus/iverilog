/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: pform.cc,v 1.21 1999/05/31 15:45:59 steve Exp $"
#endif

# include  "pform.h"
# include  "parse_misc.h"
# include  "PUdp.h"
# include  <list>
# include  <map>
# include  <assert.h>
# include  <typeinfo>
# include  <strstream>

/*
 * The lexor accesses the vl_* variables.
 */
string vl_file = "";

extern int VLparse();

static Module*cur_module = 0;

static map<string,Module*> vl_modules;
static map<string,PUdp*>   vl_primitives;

/*
 * This function evaluates delay expressions. The result should be a
 * simple constant that I can interpret as an unsigned number.
 */
static unsigned long evaluate_delay(PExpr*delay)
{
      PENumber*pp = dynamic_cast<PENumber*>(delay);
      if (pp == 0) {
	    VLerror("Sorry, delay expression is too complicated.");
	    return 0;
      }

      return pp->value().as_ulong();
}

void pform_startmodule(const string&name, list<PWire*>*ports)
{
      assert( cur_module == 0 );
      cur_module = new Module(name, ports? ports->size() : 0);

      if (ports) {
	    unsigned idx = 0;
	    for (list<PWire*>::iterator cur = ports->begin()
		       ; cur != ports->end()
		       ; cur ++ ) {
		  cur_module->add_wire(*cur);
		  cur_module->ports[idx++] = *cur;
	    }

	    delete ports;
      }
}

void pform_endmodule(const string&name)
{
      assert(cur_module);
      assert(name == cur_module->get_name());
      vl_modules[name] = cur_module;
      cur_module = 0;
}

bool pform_expression_is_constant(const PExpr*ex)
{
      return ex->is_constant(cur_module);
}

void pform_make_udp(string*name, list<string>*parms,
		    list<PWire*>*decl, list<string>*table,
		    Statement*init_expr)
{
      assert(parms->size() > 0);

	/* Put the declarations into a map, so that I can check them
	   off with the parameters in the list. I will rebuild a list
	   of parameters for the PUdp object. */
      map<string,PWire*> defs;
      for (list<PWire*>::iterator cur = decl->begin()
		 ; cur != decl->end()
		 ; cur ++ )

	    if (defs[(*cur)->name] == 0) {
		  defs[(*cur)->name] = *cur;

	    } else switch ((*cur)->port_type) {

		case NetNet::PIMPLICIT:
		case NetNet::POUTPUT:
		  assert(defs[(*cur)->name]->port_type != NetNet::PINPUT);
		    // OK, merge the output definitions.
		  defs[(*cur)->name]->port_type = NetNet::POUTPUT;
		  if ((*cur)->type == NetNet::REG)
			defs[(*cur)->name]->type = NetNet::REG;
		  break;

		case NetNet::PINPUT:
		    // Allow duplicate input declarations.
		  assert(defs[(*cur)->name]->port_type == NetNet::PINPUT);
		  delete *cur;
		  break;

		default:
		  assert(0);
	    }


	/* Put the parameters into a vector of wire descriptions. Look
	   in the map for the definitions of the name. */
      vector<PWire*> pins (parms->size());
      { list<string>::iterator cur;
        unsigned idx;
        for (cur = parms->begin(), idx = 0
		   ; cur != parms->end()
		   ; idx++, cur++) {
	      pins[idx] = defs[*cur];
	}
      }

	/* Check that the output is an output and the inputs are
	   inputs. I can also make sure that only the single output is
	   declared a register, if anything. */
      assert(pins.size() > 0);
      assert(pins[0]);
      assert(pins[0]->port_type == NetNet::POUTPUT);
      for (unsigned idx = 1 ;  idx < pins.size() ;  idx += 1) {
	    assert(pins[idx]);
	    assert(pins[idx]->port_type == NetNet::PINPUT);
	    assert(pins[idx]->type != NetNet::REG);
      }

	/* Interpret and check the table entry strings, to make sure
	   they correspond to the inputs, output and output type. Make
	   up vectors for the fully interpreted result that can be
	   placed in the PUdp object. */
      vector<string> input   (table->size());
      vector<char>   current (table->size());
      vector<char>   output  (table->size());
      { unsigned idx = 0;
        for (list<string>::iterator cur = table->begin()
		   ; cur != table->end()
		   ; cur ++, idx += 1) {
	      string tmp = *cur;
	      assert(tmp.find(':') == (pins.size() - 1));

	      input[idx] = tmp.substr(0, pins.size()-1);
	      tmp = tmp.substr(pins.size()-1);

	      if (pins[0]->type == NetNet::REG) {
		    assert(tmp[0] == ':');
		    assert(tmp.size() == 4);
		    current[idx] = tmp[1];
		    tmp = tmp.substr(2);
	      }

	      assert(tmp[0] == ':');
	      assert(tmp.size() == 2);
	      output[idx] = tmp[1];
	}
      }

	/* Verify the "initial" statement, if present, to be sure that
	   it only assignes to the output and the output is
	   registered. Then save the initial value that I get. */
      verinum::V init = verinum::Vx;
      if (init_expr) {
	      // XXXX
	    assert(pins[0]->type == NetNet::REG);

	    PAssign*pa = dynamic_cast<PAssign*>(init_expr);
	    assert(pa);

	    const PEIdent*id = dynamic_cast<const PEIdent*>(pa->lval());
	    assert(id);

	      // XXXX
	    assert(id->name() == pins[0]->name);

	    const PENumber*np = dynamic_cast<const PENumber*>(pa->get_expr());
	    assert(np);

	    init = np->value()[0];
      }

	// Put the primitive into the primitives table
      if (vl_primitives[*name]) {
	    VLerror("UDP primitive already exists.");

      } else {
	    PUdp*udp = new PUdp(*name, parms->size());

	      // Detect sequential udp.
	    if (pins[0]->type == NetNet::REG)
		  udp->sequential = true;

	      // Make the port list for the UDP
	    for (unsigned idx = 0 ;  idx < pins.size() ;  idx += 1)
		  udp->ports[idx] = pins[idx]->name;

	    udp->tinput   = input;
	    udp->tcurrent = current;
	    udp->toutput  = output;
	    udp->initial  = init;

	    vl_primitives[*name] = udp;
      }

	/* Delete the excess tables and lists from the parser. */
      delete name;
      delete parms;
      delete decl;
      delete table;
      delete init_expr;
}

/*
 * pform_makegates is called when a list of gates (with the same type)
 * are ready to be instantiated. The function runs through the list of
 * gates and calls the pform_makegate function to make the individual gate.
 */
void pform_makegate(PGBuiltin::Type type,
		    unsigned long delay_val,
		    const lgate&info)
{
      if (info.parms_by_name) {
	    cerr << info.file << ":" << info.lineno << ": Gates do not "
		  "have port names." << endl;
	    error_count += 1;
	    return;
      }

      PGBuiltin*cur = new PGBuiltin(type, info.name, info.parms, delay_val);
      if (info.range[0])
	    cur->set_range(info.range[0], info.range[1]);

      cur->set_file(info.file);
      cur->set_lineno(info.lineno);

      cur_module->add_gate(cur);
}

void pform_makegates(PGBuiltin::Type type,
		     PExpr*delay, svector<lgate>*gates)
{
      unsigned long delay_val = delay? evaluate_delay(delay) : 0;
      delete delay;

      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    pform_makegate(type, delay_val, (*gates)[idx]);
      }

      delete gates;
}

/*
 * A module is different from a gate in that there are different
 * constraints, and sometimes different syntax.
 */
static void pform_make_modgate(const string&type,
			       const string&name,
			       svector<PExpr*>*wires,
			       const string&fn, unsigned ln)
{
      if (name == "") {
	    cerr << fn << ":" << ln << ": Instantiation of " << type
		 << " module requires an instance name." << endl;
	    error_count += 1;
	    return;
      }

      PGate*cur = new PGModule(type, name, wires);
      cur->set_file(fn);
      cur->set_lineno(ln);
      cur_module->add_gate(cur);
}

static void pform_make_modgate(const string&type,
			       const string&name,
			       svector<portname_t*>*bind,
			       const string&fn, unsigned ln)
{
      if (name == "") {
	    cerr << fn << ":" << ln << ": Instantiation of " << type
		 << " module requires an instance name." << endl;
	    error_count += 1;
	    return;
      }

      unsigned npins = bind->count();
      PGModule::bind_t*pins = new PGModule::bind_t[npins];
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    portname_t*curp = (*bind)[idx];
	    pins[idx].name = curp->name;
	    pins[idx].parm = curp->parm;
      }

      PGate*cur = new PGModule(type, name, pins, npins);
      cur->set_file(fn);
      cur->set_lineno(ln);
      cur_module->add_gate(cur);
}

void pform_make_modgates(const string&type, svector<lgate>*gates)
{
      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    lgate cur = (*gates)[idx];

	    if (cur.parms_by_name) {
		  pform_make_modgate(type, cur.name, cur.parms_by_name,
				     cur.file, cur.lineno);

	    } else if (cur.parms) {
		  pform_make_modgate(type, cur.name, cur.parms, cur.file,
				     cur.lineno);
	    } else {
		  svector<PExpr*>*wires = new svector<PExpr*>(0);
		  pform_make_modgate(type, cur.name, wires, cur.file,
				     cur.lineno);
	    }
      }

      delete gates;
}

PGAssign* pform_make_pgassign(PExpr*lval, PExpr*rval)
{
      svector<PExpr*>*wires = new svector<PExpr*>(2);
      (*wires)[0] = lval;
      (*wires)[1] = rval;
      PGAssign*cur = new PGAssign(wires);
      cur_module->add_gate(cur);
      return cur;
}

void pform_makewire(const string&name, NetNet::Type type)
{
      PWire*cur = cur_module->get_wire(name);
      if (cur) {
	    if (cur->type != NetNet::IMPLICIT) {
		  strstream msg;
		  msg << "Duplicate definition of " << name << ".";
		  VLerror(msg.str());
	    }
	    cur->type = type;
	    return;
      }

      cur = new PWire(name, type);
      cur_module->add_wire(cur);
}

void pform_makewire(const list<string>*names, NetNet::Type type)
{
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ )
	    pform_makewire(*cur, type);

}

void pform_set_port_type(const string&name, NetNet::PortType pt)
{
      PWire*cur = cur_module->get_wire(name);
      if (cur == 0) {
	    VLerror("name is not a port.");
	    return;
      }

      if (cur->port_type != NetNet::PIMPLICIT) {
	    VLerror("error setting port direction.");
	    return;
      }

      cur->port_type = pt;
}

void pform_set_attrib(const string&name, const string&key, const string&value)
{
      PWire*cur = cur_module->get_wire(name);
      assert(cur);
      cur->attributes[key] = value;
}

/*
 * Set the attribute of a TYPE. This is different from an object in
 * that this applies to every instantiation of the given type.
 */
void pform_set_type_attrib(const string&name, const string&key,
			   const string&value)
{
      map<string,PUdp*>::const_iterator udp = vl_primitives.find(name);
      if (udp == vl_primitives.end()) {
	    VLerror("type name is not (yet) defined.");
	    return;
      }

      (*udp).second ->attributes[key] = value;
}

void pform_set_reg_idx(const string&name, PExpr*l, PExpr*r)
{
      PWire*cur = cur_module->get_wire(name);
      if (cur == 0) {
	    VLerror("name is not a valid net.");
	    return;
      }

      assert(cur->lidx == 0);
      assert(cur->ridx == 0);
      cur->lidx = l;
      cur->ridx = r;
}

static void pform_set_net_range(const string&name, const svector<PExpr*>*range)
{
      assert(range->count() == 2);

      PWire*cur = cur_module->get_wire(name);
      if (cur == 0) {
	    VLerror("name is not a valid net.");
	    return;
      }

      if ((cur->msb == 0) && (cur->lsb == 0)){
	    cur->msb = (*range)[0];
	    cur->lsb = (*range)[1];
      } else {
	    PExpr*msb = (*range)[0];
	    PExpr*lsb = (*range)[1];
	    if (msb == 0) {
		  VLerror(yylloc, "failed to parse msb of range.");
	    } else if (lsb == 0) {
		  VLerror(yylloc, "failed to parse lsb of range.");
	    } else if (! (cur->msb->is_the_same(msb) &&
			  cur->lsb->is_the_same(lsb))) {
		  VLerror(yylloc, "net ranges are not identical.");
	    }
	      //delete msb;
	      //delete lsb;
      }
}

void pform_set_parameter(const string&name, PExpr*expr)
{
      cur_module->parameters[name] = expr;
}

void pform_set_port_type(list<string>*names, NetNet::PortType pt)
{
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    pform_set_port_type(*cur, pt);
      }
}

void pform_set_net_range(list<string>*names, const svector<PExpr*>*range)
{
      assert(range->count() == 2);

      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    pform_set_net_range(*cur, range);
      }
}

list<PWire*>* pform_make_udp_input_ports(list<string>*names)
{
      list<PWire*>*out = new list<PWire*>;

      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    PWire*pp = new PWire(*cur);
	    pp->port_type = NetNet::PINPUT;
	    out->push_back(pp);
      }

      delete names;
      return out;
}

PProcess* pform_make_behavior(PProcess::Type type, Statement*st)
{
      PProcess*pp = new PProcess(type, st);
      cur_module->add_behavior(pp);
      return pp;
}

Statement* pform_make_block(PBlock::BL_TYPE type, list<Statement*>*sl)
{
      if (sl == 0)
	    sl = new list<Statement*>;

      PBlock*bl = new PBlock(type, *sl);
      delete sl;
      return bl;
}

Statement* pform_make_calltask(string*name, svector<PExpr*>*parms)
{
      if (parms == 0)
	    parms = new svector<PExpr*>(0);

      PCallTask*ct = new PCallTask(*name, *parms);
      delete name;
      delete parms;
      return ct;
}

FILE*vl_input = 0;
int pform_parse(const char*path, map<string,Module*>&modules,
		map<string,PUdp*>&prim)
{
      vl_file = path;
      vl_input = fopen(path, "r");
      if (vl_input == 0) {
	    cerr << "Unable to open " <<vl_file << "." << endl;
	    return 11;
      }

      error_count = 0;
      warn_count = 0;
      int rc = VLparse();
      if (rc) {
	    cerr << "I give up." << endl;
      }

      modules = vl_modules;
      prim = vl_primitives;
      return error_count;
}


/*
 * $Log: pform.cc,v $
 * Revision 1.21  1999/05/31 15:45:59  steve
 *  makegates infinite loop fixed.
 *
 * Revision 1.20  1999/05/29 02:36:17  steve
 *  module parameter bind by name.
 *
 * Revision 1.19  1999/05/20 04:31:45  steve
 *  Much expression parsing work,
 *  mark continuous assigns with source line info,
 *  replace some assertion failures with Sorry messages.
 *
 * Revision 1.18  1999/05/16 05:08:42  steve
 *  Redo constant expression detection to happen
 *  after parsing.
 *
 *  Parse more operators and expressions.
 *
 * Revision 1.17  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.16  1999/05/08 20:19:20  steve
 *  Parse more things.
 *
 * Revision 1.15  1999/05/07 04:26:49  steve
 *  Parse more complex continuous assign lvalues.
 *
 * Revision 1.14  1999/05/06 04:37:17  steve
 *  Get rid of list<lgate> types.
 *
 * Revision 1.13  1999/05/06 04:09:28  steve
 *  Parse more constant expressions.
 *
 * Revision 1.12  1999/05/02 23:25:32  steve
 *  Enforce module instance names.
 *
 * Revision 1.11  1999/04/19 01:59:37  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.10  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.9  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.8  1999/01/25 05:45:56  steve
 *  Add the LineInfo class to carry the source file
 *  location of things. PGate, Statement and PProcess.
 *
 *  elaborate handles module parameter mismatches,
 *  missing or incorrect lvalues for procedural
 *  assignment, and errors are propogated to the
 *  top of the elaboration call tree.
 *
 *  Attach line numbers to processes, gates and
 *  assignment statements.
 *
 * Revision 1.7  1998/12/09 04:02:47  steve
 *  Support the include directive.
 *
 * Revision 1.6  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.5  1998/11/25 02:35:53  steve
 *  Parse UDP primitives all the way to pform.
 *
 * Revision 1.4  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.3  1998/11/11 00:01:51  steve
 *  Check net ranges in declarations.
 *
 * Revision 1.2  1998/11/07 17:05:06  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:29:03  steve
 *  Introduce verilog to CVS.
 *
 */

