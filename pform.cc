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
#ident "$Id: pform.cc,v 1.2 1998/11/07 17:05:06 steve Exp $"
#endif

# include  "pform.h"
# include  "parse_misc.h"
# include  <list>
# include  <assert.h>
# include  <typeinfo>

extern int VLparse();

static Module*cur_module = 0;

static list<Module*>*vl_modules = 0;

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
	    VLwarn(yylloc, "net ranges not checked.");
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
int pform_parse(FILE*input, list<Module*>&modules)
{
      vl_input = input;
      vl_modules = &modules;
      error_count = 0;
      warn_count = 0;
      int rc = VLparse();
      if (rc) {
	    cerr << "I give up." << endl;
      }
      return error_count;
}


/*
 * $Log: pform.cc,v $
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

