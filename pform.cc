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
#ident "$Id: pform.cc,v 1.5 1998/11/25 02:35:53 steve Exp $"
#endif

# include  "pform.h"
# include  "parse_misc.h"
# include  "PUdp.h"
# include  <list>
# include  <map>
# include  <assert.h>
# include  <typeinfo>

extern int VLparse();

static Module*cur_module = 0;

static list<Module*>*vl_modules = 0;
static map<string,PUdp*> vl_primitives;

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

      vl_modules->push_back(cur_module);

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
      cur_module = 0;
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

	      // XXXX
	    assert(pa->lval() == pins[0]->name);

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


void pform_makegate(PGBuiltin::Type type,
		    const string&name,
		    const vector<PExpr*>&wires,
		    unsigned long delay_val)
{
      PGate*cur = new PGBuiltin(type, name, wires, delay_val);
      cur_module->add_gate(cur);
}

void pform_makegates(PGBuiltin::Type type,
		     PExpr*delay, list<lgate>*gates)
{
      unsigned long delay_val = evaluate_delay(delay);
      delete delay;

      while (! gates->empty()) {
	    lgate cur = gates->front();
	    gates->pop_front();

	    vector<PExpr*>wires (cur.parms->size());
	    for (unsigned idx = 0 ;  idx < wires.size() ;  idx += 1) {
		  PExpr*ep = cur.parms->front();
		  cur.parms->pop_front();

		  wires[idx] = ep;
	    }

	    pform_makegate(type, cur.name, wires, delay_val);
      }

      delete gates;
}

void pform_make_modgate(const string&type,
			const string&name,
			const vector<PExpr*>&wires)
{
      PGate*cur = new PGModule(type, name, wires);
      cur_module->add_gate(cur);
}

void pform_make_modgates(const string&type, list<lgate>*gates)
{
      while (! gates->empty()) {
	    lgate cur = gates->front();
	    gates->pop_front();

	    vector<PExpr*>wires (cur.parms->size());
	    for (unsigned idx = 0 ;  idx < wires.size() ;  idx += 1) {
		  PExpr*ep = cur.parms->front();
		  cur.parms->pop_front();

		  wires[idx] = ep;
	    }

	    pform_make_modgate(type, cur.name, wires);
      }

      delete gates;
}

void pform_make_pgassign(const string&lval, PExpr*rval)
{
      vector<PExpr*> wires (2);
      wires[0] = new PEIdent(lval);
      wires[1] = rval;
      PGAssign*cur = new PGAssign(wires);
      cur_module->add_gate(cur);
}

void pform_make_pgassign(const string&lval, PExpr*sel, PExpr*rval)
{
      vector<PExpr*> wires (2);
      PEIdent*tmp = new PEIdent(lval);
      tmp->msb_ = sel;
      wires[0] = tmp;
      wires[1] = rval;
      PGAssign*cur = new PGAssign(wires);
      cur_module->add_gate(cur);
}

void pform_makewire(const string&name, NetNet::Type type)
{
      PWire*cur = cur_module->get_wire(name);
      if (cur) {
	    if (cur->type != NetNet::IMPLICIT)
		  VLerror("Extra definition of wire.");

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

static void pform_set_net_range(const string&name, list<PExpr*>*range)
{
      assert(range->size() == 2);

      PWire*cur = cur_module->get_wire(name);
      if (cur == 0) {
	    VLerror("name is not a valid net.");
	    return;
      }

      if ((cur->msb == 0) && (cur->lsb == 0)){
	    list<PExpr*>::const_iterator idx = range->begin();
	    cur->msb = *idx;
	    idx ++;
	    cur->lsb = *idx;
      } else {
	    list<PExpr*>::const_iterator idx = range->begin();
	    PExpr*msb = *idx;
	    idx ++;
	    PExpr*lsb = *idx;
	    if (! (cur->msb->is_the_same(msb) && cur->lsb->is_the_same(lsb)))
		  VLerror(yylloc, "net ranges are not identical.");
	    delete msb;
	    delete lsb;
      }
}

void pform_set_port_type(list<string>*names, NetNet::PortType pt)
{
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    pform_set_port_type(*cur, pt);
      }
}

void pform_set_net_range(list<string>*names, list<PExpr*>*range)
{
      assert(range->size() == 2);

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

void pform_make_behavior(PProcess::Type type, Statement*st)
{
      PProcess*pp = new PProcess(type, st);
      cur_module->add_behavior(pp);
}

Statement* pform_make_block(PBlock::BL_TYPE type, list<Statement*>*sl)
{
      if (sl == 0)
	    sl = new list<Statement*>;

      PBlock*bl = new PBlock(type, *sl);
      delete sl;
      return bl;
}

Statement* pform_make_assignment(string*text, PExpr*ex)
{
      PAssign*st = new PAssign (*text, ex);
      delete text;
      return st;
}

Statement* pform_make_calltask(string*name, list<PExpr*>*parms)
{
      if (parms == 0)
	    parms = new list<PExpr*>;

      PCallTask*ct = new PCallTask(*name, *parms);
      delete name;
      delete parms;
      return ct;
}

FILE*vl_input = 0;
int pform_parse(FILE*input, list<Module*>&modules, map<string,PUdp*>&prim)
{
      vl_input = input;
      vl_modules = &modules;
      error_count = 0;
      warn_count = 0;
      int rc = VLparse();
      if (rc) {
	    cerr << "I give up." << endl;
      }

      prim = vl_primitives;
      return error_count;
}


/*
 * $Log: pform.cc,v $
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

