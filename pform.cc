/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "compiler.h"
# include  "pform.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  "PData.h"
# include  "PEvent.h"
# include  "PUdp.h"
# include  <list>
# include  <map>
# include  <assert.h>
# include  <typeinfo>
# include  <sstream>

map<perm_string,Module*> pform_modules;
map<perm_string,PUdp*> pform_primitives;

/*
 * The lexor accesses the vl_* variables.
 */
string vl_file = "";

extern int VLparse();

static Module*pform_cur_module = 0;

static NetNet::Type pform_default_nettype = NetNet::WIRE;

/*
 * These variables track the current time scale, as well as where the
 * timescale was set. This supports warnings about tangled timescales.
 */
static int pform_time_unit = 0;
static int pform_time_prec = 0;

static char*pform_timescale_file = 0;
static unsigned pform_timescale_line = 0;

/*
 * The scope stack and the following functions handle the processing
 * of scope. As I enter a scope, the push function is called, and as I
 * leave a scope the pop function is called. Entering tasks, functions
 * and named blocks causes scope to be pushed and popped. The module
 * name is not included it this scope stack.
 *
 * The hier_name function, therefore, returns the name path of a
 * function relative the current function.
 */

static hname_t scope_stack;

void pform_push_scope(char*name)
{
      scope_stack.append(name);
}

void pform_pop_scope()
{
      char*tmp = scope_stack.remove_tail_name();
      assert(tmp);
      free(tmp);
}

static hname_t hier_name(const char*tail)
{
      hname_t name = scope_stack;
      name.append(tail);
      return name;
}

void pform_set_default_nettype(NetNet::Type type,
			       const char*file, unsigned lineno)
{
      pform_default_nettype = type;

      if (pform_cur_module) {
	    cerr << file<<":"<<lineno << ": error: "
		 << "`default_nettype directives must appear" << endl;
	    cerr << file<<":"<<lineno << ":      : "
		 << "outside module definitions. The containing" << endl;
	    cerr << file<<":"<<lineno << ":      : "
		 << "module " << pform_cur_module->mod_name()
		 << " starts on line "
		 << pform_cur_module->get_line() << "." << endl;
	    error_count += 1;
      }
}

/*
 * The lexor calls this function to set the active timescale when it
 * detects a `timescale directive. The function saves the directive
 * values (for use by modules) and if warnings are enabled checks to
 * see if some modules have no timescale.
 */
void pform_set_timescale(int unit, int prec,
			 const char*file, unsigned lineno)
{
      bool first_flag = true;

      assert(unit >= prec);
      pform_time_unit = unit;
      pform_time_prec = prec;

      if (pform_timescale_file) {
	    free(pform_timescale_file);
	    first_flag = false;
      }

      pform_timescale_file = strdup(file);
      pform_timescale_line = lineno;

      if (warn_timescale && first_flag && (pform_modules.size() > 0)) {
	    cerr << file << ":" << lineno << ": warning: "
		 << "Some modules have no timescale. This may cause"
		 << endl;
	    cerr << file << ":" << lineno << ":        : "
		 << "confusing timing results.  Affected modules are:"
		 << endl;

	    map<perm_string,Module*>::iterator mod;
	    for (mod = pform_modules.begin()
		       ; mod != pform_modules.end() ; mod++) {
		  const Module*mp = (*mod).second;

		  cerr << file << ":" << lineno << ":        : "
		       << "  -- module " << (*mod).first
		       << " declared here: " << mp->get_line() << endl;
	    }
      }
}


verinum* pform_verinum_with_size(verinum*siz, verinum*val,
				 const char*file, unsigned lineno)
{
      assert(siz->is_defined());
      unsigned long size = siz->as_ulong();

      verinum::V pad;

      switch (val->get(val->len()-1)) {
	  case verinum::Vz:
	    pad = verinum::Vz;
	    break;
	  case verinum::Vx:
	    pad = verinum::Vx;
	    break;
	  default:
	    pad = verinum::V0;
	    break;
      }

      verinum*res = new verinum(pad, size);

      unsigned copy = val->len();
      if (res->len() < copy)
	    copy = res->len();

      for (unsigned idx = 0 ;  idx < copy ;  idx += 1) {
	    res->set(idx, val->get(idx));
      }

      res->has_sign(val->has_sign());

      bool trunc_flag = false;
      for (unsigned idx = copy ;  idx < val->len() ;  idx += 1) {
	    if (val->get(idx) != pad) {
		  trunc_flag = true;
		  break;
	    }
      }

      if (trunc_flag) {
	    cerr << file << ":" << lineno << ": warning: Numeric constant "
		 << "truncated to " << copy << " bits." << endl;
      }

      delete siz;
      delete val;
      return res;
}

void pform_attach_attributes(Statement*obj, svector<named_pexpr_t*>*attr)
{
      if (obj == 0)
	    return;
      if (attr == 0)
	    return;

      for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
	    named_pexpr_t*tmp = (*attr)[idx];
	    obj->attributes[tmp->name] = tmp->parm;
      }
}

void pform_startmodule(const char*name, const char*file, unsigned lineno,
		       svector<named_pexpr_t*>*attr)
{
      assert( pform_cur_module == 0 );

      perm_string lex_name = lex_strings.make(name);
      pform_cur_module = new Module(lex_name);
      pform_cur_module->time_unit = pform_time_unit;
      pform_cur_module->time_precision = pform_time_prec;
      pform_cur_module->default_nettype = pform_default_nettype;

      pform_cur_module->set_file(file);
      pform_cur_module->set_lineno(lineno);

      if (warn_timescale && pform_timescale_file
	  && (strcmp(pform_timescale_file,file) != 0)) {

	    cerr << pform_cur_module->get_line() << ": warning: "
		 << "timescale for " << name
		 << " inherited from another file." << endl;
	    cerr << pform_timescale_file << ":" << pform_timescale_line
		 << ": ...: The inherited timescale is here." << endl;
      }
      if (attr) {
	      for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		      named_pexpr_t*tmp = (*attr)[idx];
		      pform_cur_module->attributes[tmp->name] = tmp->parm;
	      }
      }
}

/*
 * This function is called by the parser to make a simple port
 * reference. This is a name without a .X(...), so the internal name
 * should be generated to be the same as the X.
 */
Module::port_t* pform_module_port_reference(char*name,
					    const char*file,
					    unsigned lineno)
{
      Module::port_t*ptmp = new Module::port_t;
      PEIdent*tmp = new PEIdent(hname_t(name));
      tmp->set_file(file);
      tmp->set_lineno(lineno);
      ptmp->name = lex_strings.make(name);
      ptmp->expr = svector<PEIdent*>(1);
      ptmp->expr[0] = tmp;

      return ptmp;
}

void pform_module_set_ports(svector<Module::port_t*>*ports)
{
      assert(pform_cur_module);

	/* The parser parses ``module foo()'' as having one
	   unconnected port, but it is really a module with no
	   ports. Fix it up here. */
      if (ports && (ports->count() == 1) && ((*ports)[0] == 0)) {
	    delete ports;
	    ports = 0;
      }

      if (ports != 0) {
	    pform_cur_module->ports = *ports;
	    delete ports;
      }
}

void pform_endmodule(const char*name)
{
      assert(pform_cur_module);
      perm_string mod_name = pform_cur_module->mod_name();
      assert(strcmp(name, mod_name) == 0);

      map<perm_string,Module*>::const_iterator test =
	    pform_modules.find(mod_name);

      if (test != pform_modules.end()) {
	    ostringstream msg;
	    msg << "Module " << name << " was already declared here: "
		<< (*test).second->get_line() << endl;
	    VLerror(msg.str().c_str());
	    pform_cur_module = 0;
	    return;
      }
      pform_modules[mod_name] = pform_cur_module;
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
	    delete typ;
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

static void process_udp_table(PUdp*udp, list<string>*table,
			      const char*file, unsigned lineno)
{
      const bool synchronous_flag = udp->sequential;

	/* Interpret and check the table entry strings, to make sure
	   they correspond to the inputs, output and output type. Make
	   up vectors for the fully interpreted result that can be
	   placed in the PUdp object.

	   The table strings are made up by the parser to be two or
	   three substrings separated by ';', i.e.:

	   0101:1:1  (synchronous device entry)
	   0101:0    (combinational device entry)

	   The parser doesn't check that we got the right kind here,
	   so this loop must watch out. */
      svector<string> input   (table->size());
      svector<char>   current (table->size());
      svector<char>   output  (table->size());
      { unsigned idx = 0;
        for (list<string>::iterator cur = table->begin()
		   ; cur != table->end()
		   ; cur ++, idx += 1) {
	      string tmp = *cur;

		/* Pull the input values from the string. */
	      assert(tmp.find(':') == (udp->ports.count() - 1));
	      input[idx] = tmp.substr(0, udp->ports.count()-1);
	      tmp = tmp.substr(udp->ports.count()-1);

	      assert(tmp[0] == ':');

		/* If this is a synchronous device, get the current
		   output string. */
	      if (synchronous_flag) {
		    if (tmp.size() != 4) {
			  cerr << file<<":"<<lineno << ": error: "
			       << "Invalid table format for"
			       << " sequential primitive." << endl;
			  error_count += 1;
			  break;
		    }
		    assert(tmp.size() == 4);
		    current[idx] = tmp[1];
		    tmp = tmp.substr(2);

	      } else if (tmp.size() != 2) {
		  cerr << file<<":"<<lineno << ": error: "
		       << "Invalid table format for"
		       << " combinational primitive." << endl;
		  error_count += 1;
		  break;
	      }

		/* Finally, extract the desired output. */
	      assert(tmp.size() == 2);
	      output[idx] = tmp[1];
	}
      }

      udp->tinput   = input;
      udp->tcurrent = current;
      udp->toutput  = output;
}

void pform_make_udp(perm_string name, list<string>*parms,
		    svector<PWire*>*decl, list<string>*table,
		    Statement*init_expr,
		    const char*file, unsigned lineno)
{
      unsigned local_errors = 0;
      assert(parms->size() > 0);

	/* Put the declarations into a map, so that I can check them
	   off with the parameters in the list. If the port is already
	   in the map, merge the port type. I will rebuild a list
	   of parameters for the PUdp object. */
      map<string,PWire*> defs;
      for (unsigned idx = 0 ;  idx < decl->count() ;  idx += 1) {

	    hname_t pname = (*decl)[idx]->path();

	    if (PWire*cur = defs[pname.peek_name(0)]) {
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
		  defs[pname.peek_name(0)] = (*decl)[idx];
	    }
      }


	/* Put the parameters into a vector of wire descriptions. Look
	   in the map for the definitions of the name. In this loop,
	   the parms list in the list of ports in the port list of the
	   UDP declaration, and the defs map maps that name to a
	   PWire* created by an input or output declaration. */
      svector<PWire*> pins (parms->size());
      svector<string> pin_names (parms->size());
      { list<string>::iterator cur;
        unsigned idx;
        for (cur = parms->begin(), idx = 0
		   ; cur != parms->end()
		   ; idx++, cur++) {
	      pins[idx] = defs[*cur];
	      pin_names[idx] = *cur;
	}
      }

	/* Check that the output is an output and the inputs are
	   inputs. I can also make sure that only the single output is
	   declared a register, if anything. The possible errors are:

	      -- an input port (not the first) is missing an input
	         declaration.

	      -- An input port is declared output.

	*/
      assert(pins.count() > 0);
      do {
	    if (pins[0] == 0) {
		  cerr << file<<":"<<lineno << ": error: "
		       << "Output port of primitive " << name
		       << " missing output declaration." << endl;
		  cerr << file<<":"<<lineno << ":      : "
		       << "Try: output " << pin_names[0] << ";"
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  break;
	    }
	    if (pins[0]->get_port_type() != NetNet::POUTPUT) {
		  cerr << file<<":"<<lineno << ": error: "
		       << "The first port of a primitive"
		       << " must be an output." << endl;
		  cerr << file<<":"<<lineno << ":      : "
		       << "Try: output " << pin_names[0] << ";"
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  break;;
	    }
      } while (0);

      for (unsigned idx = 1 ;  idx < pins.count() ;  idx += 1) {
	    if (pins[idx] == 0) {
		  cerr << file<<":"<<lineno << ": error: "
		       << "Port " << (idx+1)
		       << " of primitive " << name << " missing"
		       << " input declaration." << endl;
		  cerr << file<<":"<<lineno << ":      : "
		       << "Try: input " << pin_names[idx] << ";"
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  continue;
	    }
	    if (pins[idx]->get_port_type() != NetNet::PINPUT) {
		  cerr << file<<":"<<lineno << ": error: "
		       << "Input port " << (idx+1)
		       << " of primitive " << name
		       << " has an output (or missing) declaration." << endl;
		  cerr << file<<":"<<lineno << ":      : "
		       << "Note that only the first port can be an output."
		       << endl;
		  cerr << file<<":"<<lineno << ":      : "
		       << "Try \"input " << name << ";\""
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  continue;
	    }

	    if (pins[idx]->get_wire_type() == NetNet::REG) {
		  cerr << file<<":"<<lineno << ": error: "
		       << "Port " << (idx+1)
		       << " of primitive " << name << " is an input port"
		       << " with a reg declaration." << endl;
		  cerr << file<<":"<<lineno << ":      : "
		       << "primitive inputs cannot be reg."
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  continue;
	    }
      }

      if (local_errors > 0) {
	    delete parms;
	    delete decl;
	    delete table;
	    delete init_expr;
	    return;
      }


	/* Verify the "initial" statement, if present, to be sure that
	   it only assigns to the output and the output is
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
	      //assert(id->name() == pins[0]->name());

	    const PENumber*np = dynamic_cast<const PENumber*>(pa->rval());
	    assert(np);

	    init = np->value()[0];
      }

	// Put the primitive into the primitives table
      if (pform_primitives[name]) {
	    VLerror("UDP primitive already exists.");

      } else {
	    PUdp*udp = new PUdp(name, parms->size());

	      // Detect sequential udp.
	    if (pins[0]->get_wire_type() == NetNet::REG)
		  udp->sequential = true;

	      // Make the port list for the UDP
	    for (unsigned idx = 0 ;  idx < pins.count() ;  idx += 1)
		  udp->ports[idx] = pins[idx]->path().peek_name(0);

	    process_udp_table(udp, table, file, lineno);
	    udp->initial  = init;

	    pform_primitives[name] = udp;
      }


	/* Delete the excess tables and lists from the parser. */
      delete parms;
      delete decl;
      delete table;
      delete init_expr;
}

void pform_make_udp(perm_string name, bool synchronous_flag,
		    perm_string out_name, PExpr*init_expr,
		    list<perm_string>*parms, list<string>*table,
		    const char*file, unsigned lineno)
{

      svector<PWire*> pins(parms->size() + 1);

	/* Make the PWire for the output port. */
      pins[0] = new PWire(hier_name(out_name),
			  synchronous_flag? NetNet::REG : NetNet::WIRE,
			  NetNet::POUTPUT);
      pins[0]->set_file(file);
      pins[0]->set_lineno(lineno);

	/* Make the PWire objects for the input ports. */
      { list<perm_string>::iterator cur;
        unsigned idx;
        for (cur = parms->begin(), idx = 1
		   ;  cur != parms->end()
		   ;  idx += 1, cur++) {
	      assert(idx < pins.count());
	      pins[idx] = new PWire(hier_name(*cur),
				    NetNet::WIRE,
				    NetNet::PINPUT);
	      pins[idx]->set_file(file);
	      pins[idx]->set_lineno(lineno);
	}
	assert(idx == pins.count());
      }

	/* Verify the initial expression, if present, to be sure that
	   it only assigns to the output and the output is
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
	      //assert(id->name() == pins[0]->name());

	    const PENumber*np = dynamic_cast<const PENumber*>(pa->rval());
	    assert(np);

	    init = np->value()[0];
      }

	// Put the primitive into the primitives table
      if (pform_primitives[name]) {
	    VLerror("UDP primitive already exists.");

      } else {
	    PUdp*udp = new PUdp(name, pins.count());

	      // Detect sequential udp.
	    udp->sequential = synchronous_flag;

	      // Make the port list for the UDP
	    for (unsigned idx = 0 ;  idx < pins.count() ;  idx += 1)
		  udp->ports[idx] = pins[idx]->path().peek_name(0);

	    assert(udp);
	    assert(table);
	    process_udp_table(udp, table, file, lineno);
	    udp->initial  = init;

	    pform_primitives[name] = udp;
      }

      delete parms;
      delete table;
      delete init_expr;
}

/*
 * This function attaches a range to a given name. The function is
 * only called by the parser within the scope of the net declaration,
 * and the name that I receive only has the tail component.
 */
static void pform_set_net_range(const char* name,
				const svector<PExpr*>*range,
				bool signed_flag)
{

      PWire*cur = pform_cur_module->get_wire(hier_name(name));
      if (cur == 0) {
	    VLerror("error: name is not a valid net.");
	    return;
      }

      if (range == 0) {
	      /* This is the special case that we really mean a
		 scalar. Set a fake range. */
	    cur->set_range(0, 0);

      } else {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    cur->set_range((*range)[0], (*range)[1]);
      }
      cur->set_signed(signed_flag);
}

void pform_set_net_range(list<perm_string>*names,
			 svector<PExpr*>*range,
			 bool signed_flag)
{
      assert((range == 0) || (range->count() == 2));

      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_set_net_range(txt, range, signed_flag);
      }

      delete names;
      if (range)
	    delete range;
}

/*
 * This is invoked to make a named event. This is the declaration of
 * the event, and not necessarily the use of it.
 */
static void pform_make_event(perm_string name, const char*fn, unsigned ln)
{
      PEvent*event = new PEvent(name);
      event->set_file(fn);
      event->set_lineno(ln);
      pform_cur_module->events[name] = event;
}

void pform_make_events(list<perm_string>*names, const char*fn, unsigned ln)
{
      list<perm_string>::iterator cur;
      for (cur = names->begin() ;  cur != names->end() ;  cur++) {
	    perm_string txt = *cur;
	    pform_make_event(txt, fn, ln);
      }

      delete names;
}

static void pform_make_datum(perm_string name, const char*fn, unsigned ln)
{
      hname_t hname = hier_name(name);
      PData*datum = new PData(hname);
      datum->set_file(fn);
      datum->set_lineno(ln);
      pform_cur_module->datum[hname] = datum;
}

void pform_make_reals(list<perm_string>*names, const char*fn, unsigned ln)
{
      list<perm_string>::iterator cur;
      for (cur = names->begin() ;  cur != names->end() ;  cur++) {
	    perm_string txt = *cur;
	    pform_make_datum(txt, fn, ln);
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
		    const lgate&info,
		    svector<named_pexpr_t*>*attr)
{
      if (info.parms_by_name) {
	    cerr << info.file << ":" << info.lineno << ": Gates do not "
		  "have port names." << endl;
	    error_count += 1;
	    return;
      }

      perm_string dev_name = lex_strings.make(info.name);
      PGBuiltin*cur = new PGBuiltin(type, dev_name, info.parms, delay);
      if (info.range[0])
	    cur->set_range(info.range[0], info.range[1]);

      if (attr) {
	    for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		  named_pexpr_t*tmp = (*attr)[idx];
		  cur->attributes[tmp->name] = tmp->parm;
	    }
      }

      cur->strength0(str.str0);
      cur->strength1(str.str1);
      cur->set_file(info.file);
      cur->set_lineno(info.lineno);

      pform_cur_module->add_gate(cur);
}

void pform_makegates(PGBuiltin::Type type,
		     struct str_pair_t str,
		     svector<PExpr*>*delay,
		     svector<lgate>*gates,
		     svector<named_pexpr_t*>*attr)
{
      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    pform_makegate(type, str, delay, (*gates)[idx], attr);
      }

      if (attr) {
	    for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		  named_pexpr_t*cur = (*attr)[idx];
		  delete cur;
	    }
	    delete attr;
      }

      delete gates;
}

/*
 * A module is different from a gate in that there are different
 * constraints, and sometimes different syntax. The X_modgate
 * functions handle the instantiations of modules (and UDP objects) by
 * making PGModule objects.
 */
static void pform_make_modgate(perm_string type,
			       perm_string name,
			       struct parmvalue_t*overrides,
			       svector<PExpr*>*wires,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      PGModule*cur = new PGModule(type, name, wires);
      cur->set_file(fn);
      cur->set_lineno(ln);
      cur->set_range(msb,lsb);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->count();
	    named<PExpr*>*byname = new named<PExpr*>[cnt];

	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1) {
		  named_pexpr_t*curp = (*overrides->by_name)[idx];
		  byname[idx].name = curp->name;
		  byname[idx].parm = curp->parm;
	    }

	    cur->set_parameters(byname, cnt);

      } else if (overrides && overrides->by_order) {
	    cur->set_parameters(overrides->by_order);
      }

      pform_cur_module->add_gate(cur);
}

static void pform_make_modgate(perm_string type,
			       perm_string name,
			       struct parmvalue_t*overrides,
			       svector<named_pexpr_t*>*bind,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      unsigned npins = bind->count();
      named<PExpr*>*pins = new named<PExpr*>[npins];
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    named_pexpr_t*curp = (*bind)[idx];
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
		  named_pexpr_t*curp = (*overrides->by_name)[idx];
		  byname[idx].name = curp->name;
		  byname[idx].parm = curp->parm;
	    }

	    cur->set_parameters(byname, cnt);

      } else if (overrides && overrides->by_order) {

	    cur->set_parameters(overrides->by_order);
      }

      pform_cur_module->add_gate(cur);
}

void pform_make_modgates(perm_string type,
			 struct parmvalue_t*overrides,
			 svector<lgate>*gates)
{

      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    lgate cur = (*gates)[idx];
	    perm_string cur_name = lex_strings.make(cur.name);

	    if (cur.parms_by_name) {
		  pform_make_modgate(type, cur_name, overrides,
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
		  pform_make_modgate(type, cur_name, overrides,
				     cur.parms,
				     cur.range[0], cur.range[1],
				     cur.file, cur.lineno);

	    } else {
		  svector<PExpr*>*wires = new svector<PExpr*>(0);
		  pform_make_modgate(type, cur_name, overrides,
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
 * This is equivalent to the combination of statements:
 *
 *    reg foo;
 *    initial foo = <expr>;
 *
 * and that is how it is parsed. This syntax is not part of the
 * IEEE1364-1995 standard, but is approved by OVI as enhancement
 * BTF-B14.
 */
void pform_make_reginit(const struct vlltype&li,
			const char*name, PExpr*expr)
{
      const hname_t sname = hier_name(name);
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

/*
 * This function is used by the parser when I have port definition of
 * the form like this:
 *
 *     input wire signed [7:0] nm;
 *
 * The port_type, type, signed_flag and range are known all at once,
 * so we can create the PWire object all at once instead of piecemeal
 * as is done for the old method.
 */
void pform_module_define_port(const struct vlltype&li,
			      const char*nm,
			      NetNet::PortType port_type,
			      NetNet::Type type,
			      bool signed_flag,
			      svector<PExpr*>*range,
			      svector<named_pexpr_t*>*attr)
{
      hname_t name = hier_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur) {
	    ostringstream msg;
	    msg << name << " definition conflicts with "
		<< "definition at " << cur->get_line()
		<< ".";
	    VLerror(msg.str().c_str());
	    return;
      }


      cur = new PWire(name, type, port_type);
      cur->set_file(li.text);
      cur->set_lineno(li.first_line);

      cur->set_signed(signed_flag);

      if (range == 0) {
	    cur->set_range(0, 0);

      } else {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    cur->set_range((*range)[0], (*range)[1]);
      }

      if (attr) {
	      for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		      named_pexpr_t*tmp = (*attr)[idx];
		      cur->attributes[tmp->name] = tmp->parm;
	      }
      }
      pform_cur_module->add_wire(cur);
}

/*
 * This function makes a single signal (a wire, a reg, etc) as
 * requested by the parser. The name is unscoped, so I attach the
 * current scope to it (with the scoped_name function) and I try to
 * resolve it with an existing PWire in the scope.
 *
 * The wire might already exist because of an implicit declaration in
 * a module port, i.e.:
 *
 *     module foo (bar...
 *
 *         reg bar;
 *
 * The output (or other port direction indicator) may or may not have
 * been seen already, so I do not do any checking with it yet. But I
 * do check to see if the name has already been declared, as this
 * function is called for every declaration.
 */
void pform_makewire(const vlltype&li, const char*nm,
		    NetNet::Type type, NetNet::PortType pt,
		    svector<named_pexpr_t*>*attr)
{
      hname_t name = hier_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur) {
	    if ((cur->get_wire_type() != NetNet::IMPLICIT)
		&& (cur->get_wire_type() != NetNet::IMPLICIT_REG)) {
		  ostringstream msg;
		  msg << name << " previously defined at " <<
			cur->get_line() << ".";
		  VLerror(msg.str().c_str());
	    } else {
		  bool rc = cur->set_wire_type(type);
		  if (rc == false) {
			ostringstream msg;
			msg << name << " definition conflicts with "
			    << "definition at " << cur->get_line()
			    << ".";
			VLerror(msg.str().c_str());
		  }
	    }

	    cur->set_file(li.text);
	    cur->set_lineno(li.first_line);
	    return;
      }

      cur = new PWire(name, type, pt);
      cur->set_file(li.text);
      cur->set_lineno(li.first_line);

      if (attr) {
	    for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		  named_pexpr_t*tmp = (*attr)[idx];
		  cur->attributes[tmp->name] = tmp->parm;
	    }
      }

      pform_cur_module->add_wire(cur);
}

void pform_makewire(const vlltype&li,
		    svector<PExpr*>*range,
		    bool signed_flag,
		    list<perm_string>*names,
		    NetNet::Type type,
		    NetNet::PortType pt,
		    svector<named_pexpr_t*>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_makewire(li, txt, type, pt, attr);
	    pform_set_net_range(txt, range, signed_flag);
      }

      delete names;
      if (range)
	    delete range;
}

void pform_makewire(const vlltype&li,
		    svector<PExpr*>*range,
		    bool signed_flag,
		    svector<PExpr*>*delay,
		    str_pair_t str,
		    net_decl_assign_t*decls,
		    NetNet::Type type)
{
      net_decl_assign_t*first = decls->next;
      decls->next = 0;

      while (first) {
	    net_decl_assign_t*next = first->next;

	    pform_makewire(li, first->name, type, NetNet::NOT_A_PORT, 0);
	    pform_set_net_range(first->name, range, signed_flag);

	    hname_t name = hier_name(first->name);
	    PWire*cur = pform_cur_module->get_wire(name);
	    if (cur != 0) {
		  PEIdent*lval = new PEIdent(hname_t(first->name));
		  lval->set_file(li.text);
		  lval->set_lineno(li.first_line);
		  pform_make_pgassign(lval, first->expr, delay, str);
	    }

	    free(first->name);
	    delete first;
	    first = next;
      }
}

void pform_set_port_type(perm_string nm, NetNet::PortType pt,
			 const char*file, unsigned lineno)
{
      hname_t name = hier_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::IMPLICIT, NetNet::PIMPLICIT);
	    cur->set_file(file);
	    cur->set_lineno(lineno);
	    pform_cur_module->add_wire(cur);
      }

      switch (cur->get_port_type()) {
	  case NetNet::PIMPLICIT:
	    if (! cur->set_port_type(pt))
		  VLerror("error setting port direction.");
	    break;

	  case NetNet::NOT_A_PORT:
	    cerr << file << ":" << lineno << ": error: "
		 << "port " << name << " is not in the port list."
		 << endl;
	    error_count += 1;
	    break;

	  default:
	    cerr << file << ":" << lineno << ": error: "
		 << "port " << name << " already has a port declaration."
		 << endl;
	    error_count += 1;
	    break;
      }

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
				      bool signed_flag,
				      svector<PExpr*>*range,
				      list<perm_string>*names,
				      const char* file,
				      unsigned lineno)
{
      assert(names);
      svector<PWire*>*res = new svector<PWire*>(0);
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; cur ++ ) {

	    perm_string txt = *cur;
	    hname_t name = hier_name(txt);

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

	    curw->set_signed(signed_flag);

	      /* If there is a range involved, it needs to be set. */
	    if (range)
		  curw->set_range((*range)[0], (*range)[1]);

	    svector<PWire*>*tmp = new svector<PWire*>(*res, curw);

	    delete res;
	    res = tmp;
      }

      if (range)
	    delete range;
      delete names;
      return res;
}

void pform_set_task(perm_string name, PTask*task)
{
      pform_cur_module->add_task(name, task);
}


void pform_set_function(perm_string name, PFunction*func)
{
      pform_cur_module->add_function(name, func);
}

void pform_set_attrib(perm_string name, perm_string key, char*value)
{
      hname_t path (name);

      if (PWire*cur = pform_cur_module->get_wire(path)) {
	    cur->attributes[key] = new PEString(value);

      } else if (PGate*cur = pform_cur_module->get_gate(name)) {
	    cur->attributes[key] = new PEString(value);

      } else {
	    free(value);
	    VLerror("Unable to match name for setting attribute.");

      }
}

/*
 * Set the attribute of a TYPE. This is different from an object in
 * that this applies to every instantiation of the given type.
 */
void pform_set_type_attrib(perm_string name, const string&key,
			   char*value)
{
      map<perm_string,PUdp*>::const_iterator udp = pform_primitives.find(name);
      if (udp == pform_primitives.end()) {
	    VLerror("type name is not (yet) defined.");
	    free(value);
	    return;
      }

      (*udp).second ->attributes[key] = new PEString(value);
}

/*
 * This function attaches a memory index range to an existing
 * register. (The named wire must be a register.
 */
void pform_set_reg_idx(const char*name, PExpr*l, PExpr*r)
{
      PWire*cur = pform_cur_module->get_wire(hier_name(name));
      if (cur == 0) {
	    VLerror("internal error: name is not a valid memory for index.");
	    return;
      }

      cur->set_memory_idx(l, r);
}

void pform_set_parameter(perm_string name, bool signed_flag,
			 svector<PExpr*>*range, PExpr*expr)
{
      assert(expr);
      pform_cur_module->parameters[name].expr = expr;

      if (range) {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    pform_cur_module->parameters[name].msb = (*range)[0];
	    pform_cur_module->parameters[name].lsb = (*range)[1];
      } else {
	    pform_cur_module->parameters[name].msb = 0;
	    pform_cur_module->parameters[name].lsb = 0;
      }
      pform_cur_module->parameters[name].signed_flag = signed_flag;

      pform_cur_module->param_names.push_back(name);
}

void pform_set_localparam(perm_string name, bool signed_flag,
			 svector<PExpr*>*range, PExpr*expr)
{
      assert(expr);
      pform_cur_module->localparams[name].expr = expr;

      if (range) {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    pform_cur_module->localparams[name].msb = (*range)[0];
	    pform_cur_module->localparams[name].lsb = (*range)[1];
      } else {
	    pform_cur_module->localparams[name].msb  = 0;
	    pform_cur_module->localparams[name].lsb  = 0;
      }
      pform_cur_module->localparams[name].signed_flag = signed_flag;
}

void pform_set_specparam(perm_string name, PExpr*expr)
{
      assert(expr);
      pform_cur_module->specparams[name] = expr;
}

void pform_set_defparam(const hname_t&name, PExpr*expr)
{
      assert(expr);
      pform_cur_module->defparms[name] = expr;
}

/*
 * XXXX Not implemented yet.
 */
extern void pform_make_specify_path(list<perm_string>*src, char pol,
				    bool full_flag, list<perm_string>*dst)
{
      delete src;
      delete dst;
}

void pform_set_port_type(const struct vlltype&li,
			 list<perm_string>*names,
			 svector<PExpr*>*range,
			 bool signed_flag,
			 NetNet::PortType pt)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_set_port_type(txt, pt, li.text, li.first_line);
	    if (range)
		  pform_set_net_range(txt, range, signed_flag);
      }

      delete names;
      if (range)
	    delete range;
}

static void pform_set_reg_integer(const char*nm)
{
      hname_t name = hier_name(nm);
      PWire*cur = pform_cur_module->get_wire(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::INTEGER, NetNet::NOT_A_PORT);
	    cur->set_signed(true);
	    pform_cur_module->add_wire(cur);
      } else {
	    bool rc = cur->set_wire_type(NetNet::INTEGER);
	    assert(rc);
	    cur->set_signed(true);
      }
      assert(cur);

      cur->set_range(new PENumber(new verinum(INTEGER_WIDTH-1, INTEGER_WIDTH)),
		     new PENumber(new verinum(0UL, INTEGER_WIDTH)));
      cur->set_signed(true);
}

void pform_set_reg_integer(list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_set_reg_integer(txt);
      }
      delete names;
}

static void pform_set_reg_time(const char*nm)
{
      hname_t name = hier_name(nm);
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

void pform_set_reg_time(list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_set_reg_time(txt);
      }
      delete names;
}

svector<PWire*>* pform_make_udp_input_ports(list<perm_string>*names)
{
      svector<PWire*>*out = new svector<PWire*>(names->size());

      unsigned idx = 0;
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    PWire*pp = new PWire(hname_t(txt),
				 NetNet::IMPLICIT,
				 NetNet::PINPUT);
	    (*out)[idx] = pp;
	    idx += 1;
      }

      delete names;
      return out;
}

PProcess* pform_make_behavior(PProcess::Type type, Statement*st,
			      svector<named_pexpr_t*>*attr)
{
      PProcess*pp = new PProcess(type, st);

      if (attr) {
	    for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		  named_pexpr_t*tmp = (*attr)[idx];
		  pp->attributes[tmp->name] = tmp->parm;
	    }
	    delete attr;
      }

      pform_cur_module->add_behavior(pp);
      return pp;
}


FILE*vl_input = 0;
extern void reset_lexor();

int pform_parse(const char*path, FILE*file)
{
      vl_file = path;
      if (file == 0) {

	    if (strcmp(path, "-") == 0)
		  vl_input = stdin;
	    else
		  vl_input = fopen(path, "r");
	    if (vl_input == 0) {
		  cerr << "Unable to open " <<vl_file << "." << endl;
		  return 11;
	    }

      } else {
	    vl_input = file;
      }

      reset_lexor();
      error_count = 0;
      warn_count = 0;
      int rc = VLparse();

      if (file == 0)
	    fclose(vl_input);

      if (rc) {
	    cerr << "I give up." << endl;
	    error_count += 1;
      }

      return error_count;
}
