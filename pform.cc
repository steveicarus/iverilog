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
#ident "$Id: pform.cc,v 1.45 1999/09/21 00:58:33 steve Exp $"
#endif

# include  "compiler.h"
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

static Module*pform_cur_module = 0;

/*
 * The scope stack and the following functions handle the processing
 * of scope. As I enter a scope, the push function is called, and as I
 * leave a scope the opo function is called.
 */
struct scope_name_t {
      string name;
      struct scope_name_t*next;
};
static scope_name_t*scope_stack  = 0;

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
 * pform_makegates is called when a list of gates (with the same type)
 * are ready to be instantiated. The function runs through the list of
 * gates and calls the pform_makegate function to make the individual gate.
 */
void pform_makegate(PGBuiltin::Type type,
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

      cur->set_file(info.file);
      cur->set_lineno(info.lineno);

      pform_cur_module->add_gate(cur);
}

void pform_makegates(PGBuiltin::Type type,
		     svector<PExpr*>*delay, svector<lgate>*gates)
{
      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    pform_makegate(type, delay, (*gates)[idx]);
      }

      delete gates;
}

/*
 * A module is different from a gate in that there are different
 * constraints, and sometimes different syntax.
 */
static void pform_make_modgate(const string&type,
			       const string&name,
			       svector<PExpr*>*overrides,
			       svector<PExpr*>*wires,
			       const string&fn, unsigned ln)
{
      if (name == "") {
	    cerr << fn << ":" << ln << ": Instantiation of " << type
		 << " module requires an instance name." << endl;
	    error_count += 1;
	    return;
      }

      PGate*cur = new PGModule(type, name, overrides, wires);
      cur->set_file(fn);
      cur->set_lineno(ln);
      pform_cur_module->add_gate(cur);
}

static void pform_make_modgate(const string&type,
			       const string&name,
			       svector<PExpr*>*overrides,
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

      PGate*cur = new PGModule(type, name, overrides, pins, npins);
      cur->set_file(fn);
      cur->set_lineno(ln);
      pform_cur_module->add_gate(cur);
}

void pform_make_modgates(const string&type,
			 svector<PExpr*>*overrides,
			 svector<lgate>*gates)
{
      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    lgate cur = (*gates)[idx];

	    if (cur.parms_by_name) {
		  pform_make_modgate(type, cur.name, overrides, cur.parms_by_name,
				     cur.file, cur.lineno);

	    } else if (cur.parms) {
		  pform_make_modgate(type, cur.name, overrides, cur.parms, cur.file,
				     cur.lineno);
	    } else {
		  svector<PExpr*>*wires = new svector<PExpr*>(0);
		  pform_make_modgate(type, cur.name, overrides, wires, cur.file,
				     cur.lineno);
	    }
      }

      delete gates;
}

PGAssign* pform_make_pgassign(PExpr*lval, PExpr*rval,
			      svector<PExpr*>*del)
{
      svector<PExpr*>*wires = new svector<PExpr*>(2);
      (*wires)[0] = lval;
      (*wires)[1] = rval;

      PGAssign*cur;

      if (del == 0)
	    cur = new PGAssign(wires);
      else
	    cur = new PGAssign(wires, del);

      pform_cur_module->add_gate(cur);
      return cur;
}

void pform_make_pgassign_list(svector<PExpr*>*alist,
			      svector<PExpr*>*del,
			      const string& text,
			      unsigned lineno)
{
	PGAssign*tmp;
	for (unsigned idx = 0 ;  idx < alist->count()/2 ;  idx += 1) {
	      tmp = pform_make_pgassign((*alist)[2*idx],
					(*alist)[2*idx+1], del);
	      tmp->set_file(text);
	      tmp->set_lineno(lineno);
	}
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
			cur->get_line() << ".";
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

void pform_makewire(const vlltype&li, const list<string>*names,
		    NetNet::Type type)
{
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ )
	    pform_makewire(li, *cur, type);

}

void pform_set_port_type(const string&name, NetNet::PortType pt)
{
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    VLerror("name is not a port.");
	    return;
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
				      const svector<PExpr*>*range,
				      const list<string>*names,
				      const string& file,
				      unsigned lineno)
{
      assert(names);
      svector<PWire*>*res = new svector<PWire*>(0);
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end() ; cur ++ ) {

	    string name = scoped_name(*cur);

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
	    delete res;
	    res = tmp;
      }

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

/*
 * This function attaches a range to a given name. The function is
 * only called by the parser within the scope of the net declaration,
 * and the name that I receive only has the tail component.
 */
static void pform_set_net_range(const string&name, const svector<PExpr*>*range)
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

void pform_set_parameter(const string&name, PExpr*expr)
{
      pform_cur_module->parameters[name] = expr;
      pform_cur_module->param_names.push_back(name);
}

void pform_set_port_type(list<string>*names, NetNet::PortType pt)
{
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    pform_set_port_type(*cur, pt);
      }
}

static void pform_set_reg_integer(const string&nm)
{
      string name = scoped_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    cur = new PWire(nm, NetNet::INTEGER, NetNet::NOT_A_PORT);
	    pform_cur_module->add_wire(cur);
      } else {
	    bool rc = cur->set_wire_type(NetNet::INTEGER);
	    assert(rc);
      }
      assert(cur);

      cur->set_range(new PENumber(new verinum(INTEGER_WIDTH-1, INTEGER_WIDTH)),
		     new PENumber(new verinum(0UL, INTEGER_WIDTH)));
}

void pform_set_reg_integer(list<string>*names)
{
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    pform_set_reg_integer(*cur);
      }
}

svector<PWire*>* pform_make_udp_input_ports(list<string>*names)
{
      svector<PWire*>*out = new svector<PWire*>(names->size());

      unsigned idx = 0;
      for (list<string>::const_iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    PWire*pp = new PWire(*cur, NetNet::IMPLICIT, NetNet::PINPUT);
	    (*out)[idx] = pp;
	    idx += 1;
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
 *
 * Revision 1.36  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.35  1999/07/31 19:15:21  steve
 *  misspelled comment.
 *
 * Revision 1.34  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.33  1999/07/24 02:11:20  steve
 *  Elaborate task input ports.
 *
 * Revision 1.32  1999/07/10 01:03:18  steve
 *  remove string from lexical phase.
 *
 * Revision 1.31  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.30  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.29  1999/06/21 01:02:16  steve
 *  Fix merging of UDP port type in decls.
 *
 * Revision 1.28  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 * Revision 1.27  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.26  1999/06/13 23:51:16  steve
 *  l-value part select for procedural assignments.
 *
 * Revision 1.25  1999/06/12 20:35:27  steve
 *  parse more verilog.
 *
 * Revision 1.24  1999/06/12 03:42:17  steve
 *  Assert state of bit range expressions.
 *
 * Revision 1.23  1999/06/06 20:45:39  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.22  1999/06/02 15:38:46  steve
 *  Line information with nets.
 *
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

