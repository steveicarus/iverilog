/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: pform.cc,v 1.70 2001/01/06 06:31:59 steve Exp $"
#endif

# include  "compiler.h"
# include  "pform.h"
# include  "parse_misc.h"
# include  "PEvent.h"
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

static Module*pform_cur_module = 0;
static int pform_time_unit = 0;
static int pform_time_prec = 0;

/*
 * The scope stack and the following functions handle the processing
 * of scope. As I enter a scope, the push function is called, and as I
 * leave a scope the pop function is called.
 *
 * The top module is not included in the scope list.
 */
struct scope_name_t {
      string name;
      struct scope_name_t*next;
};
static scope_name_t*scope_stack  = 0;

void pform_set_timescale(int unit, int prec)
{
      assert(unit >= prec);
      pform_time_unit = unit;
      pform_time_prec = prec;
}

void pform_push_scope(const string&name)
{
      scope_name_t*cur = new scope_name_t;
      cur->name = name;
      cur->next = scope_stack;
      scope_stack = cur;
}

void pform_pop_scope()
{
      assert(scope_stack);
      scope_name_t*cur = scope_stack;
      scope_stack = cur->next;
      delete cur;
}

static string scoped_name(string name)
{
      scope_name_t*cur = scope_stack;
      while (cur) {
	    name = cur->name + "." + name;
	    cur = cur->next;
      }
      return name;
}

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

void pform_startmodule(const string&name, svector<Module::port_t*>*ports)
{
      assert( pform_cur_module == 0 );

	/* The parser parses ``module foo()'' as having one
	   unconnected port, but it is really a module with no
	   ports. Fix it up here. */
      if (ports && (ports->count() == 1) && ((*ports)[0] == 0)) {
	    delete ports;
	    ports = 0;
      }

      pform_cur_module = new Module(name, ports);
      pform_cur_module->time_unit = pform_time_unit;
      pform_cur_module->time_precision = pform_time_prec;
      delete ports;
}

void pform_endmodule(const string&name)
{
      assert(pform_cur_module);
      assert(name == pform_cur_module->get_name());
      vl_modules[name] = pform_cur_module;
      pform_cur_module = 0;
}

bool pform_expression_is_constant(const PExpr*ex)
{
      return ex->is_constant(pform_cur_module);
}

MIN_TYP_MAX min_typ_max_flag = TYP;
unsigned min_typ_max_warn = 10;

PExpr* pform_select_mtm_expr(PExpr*min, PExpr*typ, PExpr*max)
{
      PExpr*res = 0;

      switch (min_typ_max_flag) {
	  case MIN:
	    res = min;
	    delete typ;
	    delete max;
	    break;
	  case TYP:
	    res = typ;
	    delete min;
	    delete max;
	    break;
	  case MAX:
	    res = max;
	    delete min;
	    delete max;
	    break;
      }

      if (min_typ_max_warn > 0) {
	    cerr << res->get_line() << ": warning: choosing ";
	    switch (min_typ_max_flag) {
		case MIN:
		  cerr << "min";
		  break;
		case TYP:
		  cerr << "typ";
		  break;
		case MAX:
		  cerr << "max";
		  break;
	    }

	    cerr << " expression." << endl;
	    min_typ_max_warn -= 1;
      }

      return res;
}

void pform_make_udp(const char*name, list<string>*parms,
		    svector<PWire*>*decl, list<string>*table,
		    Statement*init_expr)
{
      assert(parms->size() > 0);

	/* Put the declarations into a map, so that I can check them
	   off with the parameters in the list. If the port is already
	   in the map, merge the port type. I will rebuild a list
	   of parameters for the PUdp object. */
      map<string,PWire*> defs;
      for (unsigned idx = 0 ;  idx < decl->count() ;  idx += 1) {

	    string pname = (*decl)[idx]->name();
	    PWire*cur = defs[pname];
	    if (PWire*cur = defs[pname]) {
		  bool rc = true;
		  assert((*decl)[idx]);
		  if ((*decl)[idx]->get_port_type() != NetNet::PIMPLICIT) {
			rc = cur->set_port_type((*decl)[idx]->get_port_type());
			assert(rc);
		  }
		  if ((*decl)[idx]->get_wire_type() != NetNet::IMPLICIT) {
			rc = cur->set_wire_type((*decl)[idx]->get_wire_type());
			assert(rc);
		  }

	    } else {
		  defs[pname] = (*decl)[idx];
	    }
      }


	/* Put the parameters into a vector of wire descriptions. Look
	   in the map for the definitions of the name. */
      svector<PWire*> pins (parms->size());
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
      assert(pins.count() > 0);
      assert(pins[0]);
      assert(pins[0]->get_port_type() == NetNet::POUTPUT);
      for (unsigned idx = 1 ;  idx < pins.count() ;  idx += 1) {
	    assert(pins[idx]);
	    assert(pins[idx]->get_port_type() == NetNet::PINPUT);
	    assert(pins[idx]->get_wire_type() != NetNet::REG);
      }

	/* Interpret and check the table entry strings, to make sure
	   they correspond to the inputs, output and output type. Make
	   up vectors for the fully interpreted result that can be
	   placed in the PUdp object. */
      svector<string> input   (table->size());
      svector<char>   current (table->size());
      svector<char>   output  (table->size());
      { unsigned idx = 0;
        for (list<string>::iterator cur = table->begin()
		   ; cur != table->end()
		   ; cur ++, idx += 1) {
	      string tmp = *cur;
	      assert(tmp.find(':') == (pins.count() - 1));

	      input[idx] = tmp.substr(0, pins.count()-1);
	      tmp = tmp.substr(pins.count()-1);

	      if (pins[0]->get_wire_type() == NetNet::REG) {
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
	    assert(pins[0]->get_wire_type() == NetNet::REG);

	    PAssign*pa = dynamic_cast<PAssign*>(init_expr);
	    assert(pa);

	    const PEIdent*id = dynamic_cast<const PEIdent*>(pa->lval());
	    assert(id);

	      // XXXX
	    assert(id->name() == pins[0]->name());

	    const PENumber*np = dynamic_cast<const PENumber*>(pa->rval());
	    assert(np);

	    init = np->value()[0];
      }

	// Put the primitive into the primitives table
      if (vl_primitives[name]) {
	    VLerror("UDP primitive already exists.");

      } else {
	    PUdp*udp = new PUdp(name, parms->size());

	      // Detect sequential udp.
	    if (pins[0]->get_wire_type() == NetNet::REG)
		  udp->sequential = true;

	      // Make the port list for the UDP
	    for (unsigned idx = 0 ;  idx < pins.count() ;  idx += 1)
		  udp->ports[idx] = pins[idx]->name();

	    udp->tinput   = input;
	    udp->tcurrent = current;
	    udp->toutput  = output;
	    udp->initial  = init;

	    vl_primitives[name] = udp;
      }

	/* Delete the excess tables and lists from the parser. */
      delete parms;
      delete decl;
      delete table;
      delete init_expr;
}

/*
 * This function attaches a range to a given name. The function is
 * only called by the parser within the scope of the net declaration,
 * and the name that I receive only has the tail component.
 */
static void pform_set_net_range(const char*name,
				const svector<PExpr*>*range,
				bool signed_flag)
{
      assert(range);
      assert(range->count() == 2);

      PWire*cur = pform_cur_module->get_wire(scoped_name(name));
      if (cur == 0) {
	    VLerror(" error: name is not a valid net.");
	    return;
      }

      assert((*range)[0]);
      assert((*range)[1]);
      cur->set_range((*range)[0], (*range)[1]);
      cur->set_signed(signed_flag);
}

void pform_set_net_range(list<char*>*names,
			 svector<PExpr*>*range,
			 bool signed_flag)
{
      assert(range->count() == 2);

      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    char*txt = *cur;
	    pform_set_net_range(txt, range, signed_flag);
	    free(txt);
      }

      delete names;
      delete range;
}

/*
 * This is invoked to make a named event. This is the declaration of
 * the event, and not necessarily the use of it.
 */
static void pform_make_event(const char*name, const char*fn, unsigned ln)
{
      PEvent*event = new PEvent(name);
      event->set_file(fn);
      event->set_lineno(ln);
      pform_cur_module->events[name] = event;
}

void pform_make_events(list<char*>*names, const char*fn, unsigned ln)
{
      list<char*>::iterator cur;
      for (cur = names->begin() ;  cur != names->end() ;  cur++) {
	    char*txt = *cur;
	    pform_make_event(txt, fn, ln);
	    free(txt);
      }

      delete names;
}

/*
 * pform_makegates is called when a list of gates (with the same type)
 * are ready to be instantiated. The function runs through the list of
 * gates and calls the pform_makegate function to make the individual gate.
 */
void pform_makegate(PGBuiltin::Type type,
		    struct str_pair_t str,
		    svector<PExpr*>* delay,
		    const lgate&info)
{
      if (info.parms_by_name) {
	    cerr << info.file << ":" << info.lineno << ": Gates do not "
		  "have port names." << endl;
	    error_count += 1;
	    return;
      }

      PGBuiltin*cur = new PGBuiltin(type, info.name, info.parms, delay);
      if (info.range[0])
	    cur->set_range(info.range[0], info.range[1]);

      cur->strength0(str.str0);
      cur->strength1(str.str1);
      cur->set_file(info.file);
      cur->set_lineno(info.lineno);

      pform_cur_module->add_gate(cur);
}

void pform_makegates(PGBuiltin::Type type,
		     struct str_pair_t str,
		     svector<PExpr*>*delay,
		     svector<lgate>*gates)
{
      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    pform_makegate(type, str, delay, (*gates)[idx]);
      }

      delete gates;
}

/*
 * A module is different from a gate in that there are different
 * constraints, and sometimes different syntax. The X_modgate
 * functions handle the instantaions of modules (and UDP objects) by
 * making PGModule objects.
 */
static void pform_make_modgate(const string&type,
			       const string&name,
			       struct parmvalue_t*overrides,
			       svector<PExpr*>*wires,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      if (name == "") {
	    cerr << fn << ":" << ln << ": Instantiation of " << type
		 << " module requires an instance name." << endl;
	    error_count += 1;
	    return;
      }

      PGModule*cur = new PGModule(type, name, wires);
      cur->set_file(fn);
      cur->set_lineno(ln);
      cur->set_range(msb,lsb);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->count();
	    named<PExpr*>*byname = new named<PExpr*>[cnt];

	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
		  portname_t*curp = (*overrides->by_name)[idx];
		  byname[idx].name = curp->name;
		  byname[idx].parm = curp->parm;
	    }

	    cur->set_parameters(byname, cnt);

      } else if (overrides && overrides->by_order) {
	    cur->set_parameters(overrides->by_order);
      }

      pform_cur_module->add_gate(cur);
}

static void pform_make_modgate(const string&type,
			       const string&name,
			       struct parmvalue_t*overrides,
			       svector<portname_t*>*bind,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      if (name == "") {
	    cerr << fn << ":" << ln << ": Instantiation of " << type
		 << " module requires an instance name." << endl;
	    error_count += 1;
	    return;
      }

      unsigned npins = bind->count();
      named<PExpr*>*pins = new named<PExpr*>[npins];
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    portname_t*curp = (*bind)[idx];
	    pins[idx].name = curp->name;
	    pins[idx].parm = curp->parm;
      }

      PGModule*cur = new PGModule(type, name, pins, npins);
      cur->set_file(fn);
      cur->set_lineno(ln);
      cur->set_range(msb,lsb);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->count();
	    named<PExpr*>*byname = new named<PExpr*>[cnt];

	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
		  portname_t*curp = (*overrides->by_name)[idx];
		  byname[idx].name = curp->name;
		  byname[idx].parm = curp->parm;
	    }

	    cur->set_parameters(byname, cnt);

      } else if (overrides && overrides->by_order) {

	    cur->set_parameters(overrides->by_order);
      }

      pform_cur_module->add_gate(cur);
}

void pform_make_modgates(const string&type,
			 struct parmvalue_t*overrides,
			 svector<lgate>*gates)
{
      if (overrides && overrides->by_order)
	    for (unsigned idx = 0 ;  idx < overrides->by_order->count() ;  idx += 1)
		  if (! pform_expression_is_constant((*overrides->by_order)[idx])) {
			VLerror("error: Parameter override expression"
				" must be constant.");
			return;
		  }

      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    lgate cur = (*gates)[idx];

	    if (cur.parms_by_name) {
		  pform_make_modgate(type, cur.name, overrides,
				     cur.parms_by_name,
				     cur.range[0], cur.range[1],
				     cur.file, cur.lineno);

	    } else if (cur.parms) {

		    /* If there are no parameters, the parser will be
		       tricked into thinking it is one empty
		       parameter. This fixes that. */
		  if ((cur.parms->count() == 1) && (cur.parms[0][0] == 0)) {
			delete cur.parms;
			cur.parms = new svector<PExpr*>(0);
		  }
		  pform_make_modgate(type, cur.name, overrides,
				     cur.parms,
				     cur.range[0], cur.range[1],
				     cur.file, cur.lineno);

	    } else {
		  svector<PExpr*>*wires = new svector<PExpr*>(0);
		  pform_make_modgate(type, cur.name, overrides,
				     wires,
				     cur.range[0], cur.range[1],
				     cur.file, cur.lineno);
	    }
      }

      delete gates;
}

PGAssign* pform_make_pgassign(PExpr*lval, PExpr*rval,
			      svector<PExpr*>*del,
			      struct str_pair_t str)
{
      svector<PExpr*>*wires = new svector<PExpr*>(2);
      (*wires)[0] = lval;
      (*wires)[1] = rval;

      PGAssign*cur;

      if (del == 0)
	    cur = new PGAssign(wires);
      else
	    cur = new PGAssign(wires, del);

      cur->strength0(str.str0);
      cur->strength1(str.str1);

      pform_cur_module->add_gate(cur);
      return cur;
}

void pform_make_pgassign_list(svector<PExpr*>*alist,
			      svector<PExpr*>*del,
			      struct str_pair_t str,
			      const char* fn,
			      unsigned lineno)
{
	PGAssign*tmp;
	for (unsigned idx = 0 ;  idx < alist->count()/2 ;  idx += 1) {
	      tmp = pform_make_pgassign((*alist)[2*idx],
					(*alist)[2*idx+1],
					del, str);
	      tmp->set_file(fn);
	      tmp->set_lineno(lineno);
	}
}

/*
 * this function makes the initial assignment to a register as given
 * in the source. It handles the case where a reg variable is assigned
 * where it it declared:
 *
 *    reg foo = <expr>;
 *
 * This is equivilent to the combination of statements:
 *
 *    reg foo;
 *    initital foo = <expr>;
 *
 * and that is how it is parsed. This syntax is not part of the
 * IEEE1364-1994 standard, but is approved by OVI as enhancement
 * BTF-B14.
 */
void pform_make_reginit(const struct vlltype&li,
			const string&name, PExpr*expr)
{
      const string sname = scoped_name(name);
      PWire*cur = pform_cur_module->get_wire(sname);
      if (cur == 0) {
	    VLerror(li, "internal error: reginit to non-register?");
	    delete expr;
	    return;
      }

      PEIdent*lval = new PEIdent(sname);
      lval->set_file(li.text);
      lval->set_lineno(li.first_line);
      PAssign*ass = new PAssign(lval, expr);
      ass->set_file(li.text);
      ass->set_lineno(li.first_line);
      PProcess*top = new PProcess(PProcess::PR_INITIAL, ass);
      top->set_file(li.text);
      top->set_lineno(li.first_line);

      pform_cur_module->add_behavior(top);
}

void pform_makewire(const vlltype&li, const string&nm,
		    NetNet::Type type)
{
      const string name = scoped_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur) {
	    if (cur->get_wire_type() != NetNet::IMPLICIT) {
		  strstream msg;
		  msg << name << " previously defined at " <<
			cur->get_line() << "." << ends;
		  VLerror(msg.str());
	    } else {
		  bool rc = cur->set_wire_type(type);
		  assert(rc);
	    }
	    return;
      }

      cur = new PWire(name, type, NetNet::NOT_A_PORT);
      cur->set_file(li.text);
      cur->set_lineno(li.first_line);
      pform_cur_module->add_wire(cur);
}

void pform_makewire(const vlltype&li,
		    svector<PExpr*>*range,
		    list<char*>*names,
		    NetNet::Type type)
{
      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    char*txt = *cur;
	    pform_makewire(li, txt, type);
	    if (range)
		  pform_set_net_range(txt, range, false);
	    free(txt);
      }

      delete names;
      if (range)
	    delete range;
}

void pform_set_port_type(const string&nm, NetNet::PortType pt)
{
      const string name = scoped_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::IMPLICIT, pt);
	    pform_cur_module->add_wire(cur);
      }

      if (! cur->set_port_type(pt))
	    VLerror("error setting port direction.");
}

/*
 * This function is called by the parser to create task ports. The
 * resulting wire (which should be a register) is put into a list to
 * be packed into the task parameter list.
 *
 * It is possible that the wire (er, register) was already created,
 * but we know that if the name matches it is a part of the current
 * task, so in that case I just assign direction to it.
 *
 * The following example demonstrates some of the issues:
 *
 *   task foo;
 *      input a;
 *      reg a, b;
 *      input b;
 *      [...]
 *   endtask
 *
 * This function is called when the parser matches the "input a" and
 * the "input b" statements. For ``a'', this function is called before
 * the wire is declared as a register, so I create the foo.a
 * wire. For ``b'', I will find that there is already a foo.b and I
 * just set the port direction. In either case, the ``reg a, b''
 * statement is caught by the block_item non-terminal and processed
 * there.
 *
 * Ports are implicitly type reg, because it must be possible for the
 * port to act as an l-value in a procedural assignment. It is obvious
 * for output and inout ports that the type is reg, because the task
 * only contains behavior (no structure) to a procedural assignment is
 * the *only* way to affect the output. It is less obvious for input
 * ports, but in practice an input port receives its value as if by a
 * procedural assignment from the calling behavior.
 *
 * This function also handles the input ports of function
 * definitions. Input ports to function definitions have the same
 * constraints as those of tasks, so this works fine. Functions have
 * no output or inout ports.
 */
svector<PWire*>*pform_make_task_ports(NetNet::PortType pt,
				      svector<PExpr*>*range,
				      list<char*>*names,
				      const char* file,
				      unsigned lineno)
{
      assert(names);
      svector<PWire*>*res = new svector<PWire*>(0);
      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end() ; cur ++ ) {

	    char*txt = *cur;
	    string name = scoped_name(txt);

	      /* Look for a preexisting wire. If it exists, set the
		 port direction. If not, create it. */
	    PWire*curw = pform_cur_module->get_wire(name);
	    if (curw) {
		  curw->set_port_type(pt);
	    } else {
		  curw = new PWire(name, NetNet::IMPLICIT_REG, pt);
		  curw->set_file(file);
		  curw->set_lineno(lineno);
		  pform_cur_module->add_wire(curw);
	    }

	      /* If there is a range involved, it needs to be set. */
	    if (range)
		  curw->set_range((*range)[0], (*range)[1]);

	    svector<PWire*>*tmp = new svector<PWire*>(*res, curw);

	    free(txt);
	    delete res;
	    res = tmp;
      }

      if (range)
	    delete range;
      delete names;
      return res;
}

void pform_set_task(const string&name, PTask*task)
{
      pform_cur_module->add_task(name, task);
}

/*
 * This function is called to fill out the definition of the function
 * with the trappings that are discovered after the basic function
 * name is parsed.
 */
void pform_set_function(const string&name, svector<PExpr*>*ra, PFunction *func)
{
      PWire*out = new PWire(name+"."+name, NetNet::REG, NetNet::POUTPUT);
      if (ra) {
	    assert(ra->count() == 2);
	    out->set_range((*ra)[0], (*ra)[1]);
	    delete ra;
      }
      pform_cur_module->add_wire(out);
      func->set_output(out);
      pform_cur_module->add_function(name, func);
}

void pform_set_attrib(const string&name, const string&key, const string&value)
{
      PWire*cur = pform_cur_module->get_wire(name);
      if (PWire*cur = pform_cur_module->get_wire(name)) {
	    cur->attributes[key] = value;

      } else if (PGate*cur = pform_cur_module->get_gate(name)) {
	    cur->attributes[key] = value;

      } else {
	    VLerror("Unable to match name for setting attribute.");

      }
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

/*
 * This function attaches a memory index range to an existing
 * register. (The named wire must be a register.
 */
void pform_set_reg_idx(const string&name, PExpr*l, PExpr*r)
{
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    VLerror(" error: name is not a valid net.");
	    return;
      }

      cur->set_memory_idx(l, r);
}

void pform_set_parameter(const string&name, PExpr*expr)
{
      assert(expr);
      pform_cur_module->parameters[name] = expr;
      pform_cur_module->param_names.push_back(name);
}

void pform_set_localparam(const string&name, PExpr*expr)
{
      assert(expr);
      pform_cur_module->localparams[name] = expr;
}

void pform_set_defparam(const string&name, PExpr*expr)
{
      assert(expr);
      pform_cur_module->defparms[name] = expr;
}

void pform_set_port_type(list<char*>*names,
			 svector<PExpr*>*range,
			 NetNet::PortType pt)
{
      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    char*txt = *cur;
	    pform_set_port_type(txt, pt);
	    if (range)
		  pform_set_net_range(txt, range, false);
	    free(txt);
      }

      delete names;
      if (range)
	    delete range;
}

static void pform_set_reg_integer(const char*nm)
{
      string name = scoped_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::REG, NetNet::NOT_A_PORT);
	    cur->set_signed(true);
	    pform_cur_module->add_wire(cur);
      } else {
	    bool rc = cur->set_wire_type(NetNet::REG);
	    assert(rc);
	    cur->set_signed(true);
      }
      assert(cur);

      cur->set_range(new PENumber(new verinum(INTEGER_WIDTH-1, INTEGER_WIDTH)),
		     new PENumber(new verinum(0UL, INTEGER_WIDTH)));
      cur->set_signed(true);
}

void pform_set_reg_integer(list<char*>*names)
{
      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    char*txt = *cur;
	    pform_set_reg_integer(txt);
	    free(txt);
      }
      delete names;
}

static void pform_set_reg_time(const char*nm)
{
      string name = scoped_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::REG, NetNet::NOT_A_PORT);
	    pform_cur_module->add_wire(cur);
      } else {
	    bool rc = cur->set_wire_type(NetNet::REG);
	    assert(rc);
      }
      assert(cur);

      cur->set_range(new PENumber(new verinum(TIME_WIDTH-1, INTEGER_WIDTH)),
		     new PENumber(new verinum(0UL, INTEGER_WIDTH)));
}

void pform_set_reg_time(list<char*>*names)
{
      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    char*txt = *cur;
	    pform_set_reg_time(txt);
	    free(txt);
      }
      delete names;
}

svector<PWire*>* pform_make_udp_input_ports(list<char*>*names)
{
      svector<PWire*>*out = new svector<PWire*>(names->size());

      unsigned idx = 0;
      for (list<char*>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    char*txt = *cur;
	    PWire*pp = new PWire(txt, NetNet::IMPLICIT, NetNet::PINPUT);
	    (*out)[idx] = pp;
	    idx += 1;
	    free(txt);
      }

      delete names;
      return out;
}

PProcess* pform_make_behavior(PProcess::Type type, Statement*st)
{
      PProcess*pp = new PProcess(type, st);
      pform_cur_module->add_behavior(pp);
      return pp;
}


FILE*vl_input = 0;
int pform_parse(const char*path, map<string,Module*>&modules,
		map<string,PUdp*>&prim)
{
      vl_file = path;
      if (strcmp(path, "-") == 0)
	    vl_input = stdin;
      else
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
	    error_count += 1;
      }

      modules = vl_modules;
      prim = vl_primitives;
      return error_count;
}


/*
 * $Log: pform.cc,v $
 * Revision 1.70  2001/01/06 06:31:59  steve
 *  declaration initialization for time variables.
 *
 * Revision 1.69  2001/01/06 02:29:36  steve
 *  Support arrays of integers.
 *
 * Revision 1.68  2000/12/11 00:31:43  steve
 *  Add support for signed reg variables,
 *  simulate in t-vvm signed comparisons.
 *
 * Revision 1.67  2000/11/30 17:31:42  steve
 *  Change LineInfo to store const C strings.
 *
 * Revision 1.66  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.65  2000/10/31 17:00:04  steve
 *  Remove C++ string from variable lists.
 *
 * Revision 1.64  2000/09/13 16:32:26  steve
 *  Error message for invalid variable list.
 *
 * Revision 1.63  2000/07/29 17:58:21  steve
 *  Introduce min:typ:max support.
 *
 * Revision 1.62  2000/07/22 22:09:04  steve
 *  Parse and elaborate timescale to scopes.
 *
 * Revision 1.61  2000/05/23 16:03:13  steve
 *  Better parsing of expressions lists will empty expressoins.
 *
 * Revision 1.60  2000/05/16 04:05:16  steve
 *  Module ports are really special PEIdent
 *  expressions, because a name can be used
 *  many places in the port list.
 *
 * Revision 1.59  2000/05/08 05:30:20  steve
 *  Deliver gate output strengths to the netlist.
 *
 * Revision 1.58  2000/05/06 15:41:57  steve
 *  Carry assignment strength to pform.
 *
 * Revision 1.57  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 * Revision 1.56  2000/03/12 17:09:41  steve
 *  Support localparam.
 *
 * Revision 1.55  2000/03/08 04:36:54  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.54  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.53  2000/02/18 05:15:03  steve
 *  Catch module instantiation arrays.
 *
 * Revision 1.52  2000/01/09 05:50:49  steve
 *  Support named parameter override lists.
 *
 * Revision 1.51  2000/01/02 01:59:28  steve
 *  Forgot to handle no overrides at all.
 *
 * Revision 1.50  2000/01/01 23:47:58  steve
 *  Fix module parameter override syntax.
 *
 * Revision 1.49  1999/12/30 19:06:14  steve
 *  Support reg initial assignment syntax.
 *
 * Revision 1.48  1999/12/11 05:45:41  steve
 *  Fix support for attaching attributes to primitive gates.
 *
 * Revision 1.47  1999/11/23 01:04:57  steve
 *  A file name of - means standard input.
 *
 * Revision 1.46  1999/09/30 01:22:37  steve
 *  Handle declaration of integers (including scope) in functions.
 *
 * Revision 1.45  1999/09/21 00:58:33  steve
 *  Get scope right when setting the net range.
 *
 * Revision 1.44  1999/09/17 02:06:26  steve
 *  Handle unconnected module ports.
 *
 * Revision 1.43  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.42  1999/09/10 05:02:09  steve
 *  Handle integers at task parameters.
 *
 * Revision 1.41  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.40  1999/08/27 15:08:37  steve
 *  continuous assignment lists.
 *
 * Revision 1.39  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.38  1999/08/23 16:48:39  steve
 *  Parameter overrides support from Peter Monta
 *  AND and XOR support wide expressions.
 *
 * Revision 1.37  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 */

