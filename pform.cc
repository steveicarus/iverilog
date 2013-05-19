/*
 * Copyright (c) 1998-2012 Stephen Williams (steve@icarus.com)
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
# include  "PEvent.h"
# include  "PUdp.h"
# include  "PGenerate.h"
# include  "PSpec.h"
# include  "discipline.h"
# include  <list>
# include  <map>
# include  <cassert>
# include  <stack>
# include  <typeinfo>
# include  <sstream>
# include  <cstring>
# include  <cstdlib>

# include  "ivl_assert.h"

map<perm_string,Module*> pform_modules;
map<perm_string,PUdp*> pform_primitives;

/*
 * The lexor accesses the vl_* variables.
 */
string vl_file = "";

extern int VLparse();

  /* This tracks the current module being processed. There can only be
     exactly one module currently being parsed, since Verilog does not
     allow nested module definitions. */
static Module*pform_cur_module = 0;

bool pform_library_flag = false;

  /* increment this for generate schemes within a module, and set it
     to zero when a new module starts. */
static unsigned scope_generate_counter = 1;

  /* This tracks the current generate scheme being processed. This is
     always within a module. */
static PGenerate*pform_cur_generate = 0;

static NetNet::Type pform_default_nettype = NetNet::WIRE;

/*
 * These variables track the current time scale, as well as where the
 * timescale was set. This supports warnings about tangled timescales.
 */
static int pform_time_unit;
static int pform_time_prec;

static char*pform_timescale_file = 0;
static unsigned pform_timescale_line;

static inline void FILE_NAME(LineInfo*obj, const char*file, unsigned lineno)
{
      obj->set_lineno(lineno);
      obj->set_file(filename_strings.make(file));
}

/*
 * The lexical_scope keeps track of the current lexical scope that is
 * being parsed. The lexical scope may stack, so the current scope may
 * have a parent, that is restored when the current scope ends.
 *
 * Items that have scoped names are put in the lexical_scope object.
 */
static PScope* lexical_scope = 0;

void pform_pop_scope()
{
      if (pform_cur_generate) {
	    assert(pform_cur_generate->lexical_scope);
	    PScope*cur = pform_cur_generate->lexical_scope;
	    pform_cur_generate->lexical_scope = cur->pscope_parent();
      } else {
	    assert(lexical_scope);
	    lexical_scope = lexical_scope->pscope_parent();
      }
}

PTask* pform_push_task_scope(const struct vlltype&loc, char*name, bool is_auto)
{
      perm_string task_name = lex_strings.make(name);

      PTask*task;
      if (pform_cur_generate) {
	    task = new PTask(task_name, pform_cur_generate->lexical_scope,
	                     is_auto);
	    FILE_NAME(task, loc);
	      // Check if the task is already in the dictionary.
	    if (pform_cur_generate->tasks.find(task->pscope_name()) !=
	        pform_cur_generate->tasks.end()) {
		  cerr << task->get_fileline() << ": error: duplicate "
		          "definition for task '" << name << "' in '"
		       << pform_cur_module->mod_name() << "' (generate)."
		       << endl;
		  error_count += 1;
	    }
	    pform_cur_generate->tasks[task->pscope_name()] = task;
	    pform_cur_generate->lexical_scope = task;
      } else {
	    task = new PTask(task_name, lexical_scope,
                             is_auto);
	    FILE_NAME(task, loc);
	      // Check if the task is already in the dictionary.
	    if (pform_cur_module->tasks.find(task->pscope_name()) !=
	        pform_cur_module->tasks.end()) {
		  cerr << task->get_fileline() << ": error: duplicate "
		          "definition for task '" << name << "' in '"
		       << pform_cur_module->mod_name() << "'." << endl;
		  error_count += 1;
	    }
	    pform_cur_module->tasks[task->pscope_name()] = task;
	    lexical_scope = task;
      }

      return task;
}

PFunction* pform_push_function_scope(const struct vlltype&loc, char*name,
                                      bool is_auto)
{
      perm_string func_name = lex_strings.make(name);

      PFunction*func;
      if (pform_cur_generate) {
	    func = new PFunction(func_name, pform_cur_generate->lexical_scope,
                                 is_auto);
	    FILE_NAME(func, loc);
	      // Check if the function is already in the dictionary.
	    if (pform_cur_generate->funcs.find(func->pscope_name()) !=
	        pform_cur_generate->funcs.end()) {
		  cerr << func->get_fileline() << ": error: duplicate "
		          "definition for function '" << name << "' in '"
		       << pform_cur_module->mod_name() << "' (generate)."
		       << endl;
		  error_count += 1;
	    }
	    pform_cur_generate->funcs[func->pscope_name()] = func;
	    pform_cur_generate->lexical_scope = func;
      } else {
	    func = new PFunction(func_name, lexical_scope,
                                 is_auto);
	    FILE_NAME(func, loc);
	      // Check if the function is already in the dictionary.
	    if (pform_cur_module->funcs.find(func->pscope_name()) !=
	        pform_cur_module->funcs.end()) {
		  cerr << func->get_fileline() << ": error: duplicate "
		          "definition for function '" << name << "' in '"
		       << pform_cur_module->mod_name() << "'." << endl;
		  error_count += 1;
	    }
	    pform_cur_module->funcs[func->pscope_name()] = func;
	    lexical_scope = func;
      }

      return func;
}

PBlock* pform_push_block_scope(char*name, PBlock::BL_TYPE bt)
{
      perm_string block_name = lex_strings.make(name);

      PBlock*block;
      if (pform_cur_generate) {
	    block = new PBlock(block_name, pform_cur_generate->lexical_scope, bt);
	    pform_cur_generate->lexical_scope = block;
      } else {
	    block = new PBlock(block_name, lexical_scope, bt);
	    lexical_scope = block;
      }

      return block;
}

void pform_bind_attributes(map<perm_string,PExpr*>&attributes,
			   svector<named_pexpr_t*>*attr)
{
      if (attr == 0)
	    return;

      for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
	    named_pexpr_t*tmp = (*attr)[idx];
	    attributes[tmp->name] = tmp->parm;
      }
      delete attr;
}

static LexicalScope*pform_get_cur_scope()
{
      if (pform_cur_generate)
	    if (pform_cur_generate->lexical_scope)
		  return pform_cur_generate->lexical_scope;
	    else
		  return pform_cur_generate;
      else
	    return lexical_scope;
}

static bool pform_at_module_level()
{
      if (pform_cur_generate)
	    if (pform_cur_generate->lexical_scope)
		  return false;
	    else
		  return true;
      else
	    if (lexical_scope->pscope_parent())
		  return false;
	    else
		  return true;
}

PWire*pform_get_wire_in_scope(perm_string name)
{
	/* Note that if we are processing a generate, then the
	   scope depth will be empty because generate schemes
	   cannot be within sub-scopes. Only directly in
	   modules. */
      return pform_get_cur_scope()->wires_find(name);
}

static void pform_put_wire_in_scope(perm_string name, PWire*net)
{
      pform_get_cur_scope()->wires[name] = net;
}

static void pform_put_behavior_in_scope(PProcess*pp)
{
      pform_get_cur_scope()->behaviors.push_back(pp);
}

void pform_put_behavior_in_scope(AProcess*pp)
{
      pform_get_cur_scope()->analog_behaviors.push_back(pp);
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
		 << pform_cur_module->get_fileline() << "." << endl;
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

      if (file) pform_timescale_file = strdup(file);
      else pform_timescale_file = 0;
      pform_timescale_line = lineno;

      if (!warn_timescale || !first_flag || !file) return;

	/* Look to see if we have any modules without a timescale. */
      bool have_no_ts = false;
      map<perm_string,Module*>::iterator mod;
      for (mod = pform_modules.begin(); mod != pform_modules.end(); mod++) {
	    const Module*mp = (*mod).second;
	    if (mp->time_from_timescale ||
	        mp->timescale_warn_done) continue;
	    have_no_ts = true;
	    break;
      }

	/* If we do then print a message for the new ones. */
      if (have_no_ts) {
	    cerr << file << ":" << lineno << ": warning: "
		 << "Some modules have no timescale. This may cause"
		 << endl;
	    cerr << file << ":" << lineno << ":        : "
		 << "confusing timing results.  Affected modules are:"
		 << endl;

	    for (mod = pform_modules.begin()
		       ; mod != pform_modules.end() ; mod++) {
		  Module*mp = (*mod).second;
		  if (mp->time_from_timescale ||
		      mp->timescale_warn_done) continue;
		  mp->timescale_warn_done = true;

		  cerr << file << ":" << lineno << ":        : "
		       << "  -- module " << (*mod).first
		       << " declared here: " << mp->get_fileline() << endl;
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

      verinum*res = new verinum(pad, size, true);

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

void pform_startmodule(const char*name, const char*file, unsigned lineno,
		       svector<named_pexpr_t*>*attr)
{
      assert( pform_cur_module == 0 );

      perm_string lex_name = lex_strings.make(name);
      pform_cur_module = new Module(lex_name);
      pform_cur_module->time_unit = pform_time_unit;
      pform_cur_module->time_precision = pform_time_prec;
	/* If we have a timescale file then the time information is from
	 * a timescale directive. */
      pform_cur_module->time_from_timescale = pform_timescale_file != 0;
      pform_cur_module->default_nettype = pform_default_nettype;

      FILE_NAME(pform_cur_module, file, lineno);
      pform_cur_module->library_flag = pform_library_flag;

      ivl_assert(*pform_cur_module, lexical_scope == 0);
      lexical_scope = pform_cur_module;

	/* The generate scheme numbering starts with *1*, not
	   zero. That's just the way it is, thanks to the standard. */
      scope_generate_counter = 1;

      if (warn_timescale && pform_timescale_file
	  && (strcmp(pform_timescale_file,file) != 0)) {

	    cerr << pform_cur_module->get_fileline() << ": warning: "
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
Module::port_t* pform_module_port_reference(perm_string name,
					    const char*file,
					    unsigned lineno)
{
      Module::port_t*ptmp = new Module::port_t;
      PEIdent*tmp = new PEIdent(name);
      FILE_NAME(tmp, file, lineno);
      ptmp->name = name;
      ptmp->expr.push_back(tmp);

      return ptmp;
}

void pform_module_set_ports(vector<Module::port_t*>*ports)
{
      assert(pform_cur_module);

	/* The parser parses ``module foo()'' as having one
	   unconnected port, but it is really a module with no
	   ports. Fix it up here. */
      if (ports && (ports->size() == 1) && ((*ports)[0] == 0)) {
	    delete ports;
	    ports = 0;
      }

      if (ports != 0) {
	    pform_cur_module->ports = *ports;
	    delete ports;
      }
}

void pform_endmodule(const char*name, bool inside_celldefine,
                     Module::UCDriveType uc_drive_def)
{
      assert(pform_cur_module);
      perm_string mod_name = pform_cur_module->mod_name();
      assert(strcmp(name, mod_name) == 0);
      pform_cur_module->is_cell = inside_celldefine;
      pform_cur_module->uc_drive = uc_drive_def;

      map<perm_string,Module*>::const_iterator test =
	    pform_modules.find(mod_name);

      if (test != pform_modules.end()) {
	    ostringstream msg;
	    msg << "Module " << name << " was already declared here: "
		<< (*test).second->get_fileline() << endl;
	    VLerror(msg.str().c_str());
      } else {
	    pform_modules[mod_name] = pform_cur_module;
      }

	// The current lexical scope should be this module by now, and
	// this module should not have a parent lexical scope.
      ivl_assert(*pform_cur_module, lexical_scope == pform_cur_module);
      lexical_scope = pform_cur_module->pscope_parent();
      ivl_assert(*pform_cur_module, lexical_scope == 0);

      pform_cur_module = 0;
}

static void pform_add_genvar(const struct vlltype&li, const perm_string&name,
                             map<perm_string,LineInfo*>&genvars)
{
      LineInfo*lni = new LineInfo();
      FILE_NAME(lni, li);
      if (genvars.find(name) != genvars.end()) {
            cerr << lni->get_fileline() << ": error: genvar '"
		 << name << "' has already been declared." << endl;
            cerr << genvars[name]->get_fileline()
                << ":        the previous declaration is here." << endl;
            error_count += 1;
            delete lni;
      } else {
            genvars[name] = lni;
      }
}

void pform_genvars(const struct vlltype&li, list<perm_string>*names)
{
      list<perm_string>::const_iterator cur;
      for (cur = names->begin(); cur != names->end() ; *cur++) {
            if (pform_cur_generate)
                  pform_add_genvar(li, *cur, pform_cur_generate->genvars);
            else
                  pform_add_genvar(li, *cur, pform_cur_module->genvars);
      }

      delete names;
}

void pform_start_generate_for(const struct vlltype&li,
			      char*ident1, PExpr*init,
			      PExpr*test,
			      char*ident2, PExpr*next)
{
      PGenerate*gen = new PGenerate(scope_generate_counter++);

      FILE_NAME(gen, li);

      gen->parent = pform_cur_generate;
      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_LOOP;

      pform_cur_generate->loop_index = lex_strings.make(ident1);
      pform_cur_generate->loop_init = init;
      pform_cur_generate->loop_test = test;
      pform_cur_generate->loop_step = next;

      delete[]ident1;
      delete[]ident2;
}

void pform_start_generate_if(const struct vlltype&li, PExpr*test)
{
      PGenerate*gen = new PGenerate(scope_generate_counter++);

      FILE_NAME(gen, li);

      gen->parent = pform_cur_generate;
      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_CONDIT;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = test;
      pform_cur_generate->loop_step = 0;
}

void pform_start_generate_else(const struct vlltype&li)
{
      assert(pform_cur_generate);
      assert(pform_cur_generate->scheme_type == PGenerate::GS_CONDIT);

      PGenerate*cur = pform_cur_generate;
      pform_endgenerate();

      PGenerate*gen = new PGenerate(scope_generate_counter++);

      FILE_NAME(gen, li);

      gen->parent = pform_cur_generate;
      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_ELSE;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = cur->loop_test;
      pform_cur_generate->loop_step = 0;
}

/*
 * The GS_CASE version of the PGenerate contains only case items. The
 * items in turn contain the generated items themselves.
 */
void pform_start_generate_case(const struct vlltype&li, PExpr*expr)
{
      PGenerate*gen = new PGenerate(scope_generate_counter++);

      FILE_NAME(gen, li);

      gen->parent = pform_cur_generate;
      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_CASE;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = expr;
      pform_cur_generate->loop_step = 0;
}

/*
 * The named block generate case.
 */
void pform_start_generate_nblock(const struct vlltype&li, char*name)
{
      PGenerate*gen = new PGenerate(scope_generate_counter++);

      FILE_NAME(gen, li);

      gen->parent = pform_cur_generate;
      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_NBLOCK;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = 0;
      pform_cur_generate->loop_step = 0;

      pform_cur_generate->scope_name = lex_strings.make(name);
      delete[]name;
}

/*
 * The generate case item is a special case schema that takes its id
 * from the case schema that it is a part of. The idea is that the
 * case schema can only instantiate exactly one item, so the items
 * need not have a unique number.
 */
void pform_generate_case_item(const struct vlltype&li, svector<PExpr*>*expr_list)
{
      assert(pform_cur_generate);
      assert(pform_cur_generate->scheme_type == PGenerate::GS_CASE);

      PGenerate*gen = new PGenerate(pform_cur_generate->id_number);

      FILE_NAME(gen, li);

      gen->parent = pform_cur_generate;
      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_CASE_ITEM;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = 0;
      pform_cur_generate->loop_step = 0;

      if (expr_list != 0) {
	    pform_cur_generate->item_test.resize(expr_list->count());
	    for (unsigned idx = 0 ; idx < expr_list->count() ; idx += 1)
		  pform_cur_generate->item_test[idx] = expr_list[0][idx];
      }
}

void pform_generate_block_name(char*name)
{
      assert(pform_cur_generate != 0);
      assert(pform_cur_generate->scope_name == 0);
      pform_cur_generate->scope_name = lex_strings.make(name);
      delete[]name;
}

void pform_endgenerate()
{
      assert(pform_cur_generate != 0);
      assert(pform_cur_module);

	// If there is no explicit block name then generate a temporary
	// name. This will be replaced by the correct name later, once
	// we know all the explicit names in the surrounding scope. If
	// the naming scheme used here is changed, PGenerate::elaborate
	// must be changed to match.
      if (pform_cur_generate->scope_name == 0) {
	    char tmp[16];
	    snprintf(tmp, sizeof tmp, "$gen%d", pform_cur_generate->id_number);
	    pform_cur_generate->scope_name = lex_strings.make(tmp);
      }

      PGenerate*cur = pform_cur_generate;
      pform_cur_generate = cur->parent;

      if (pform_cur_generate != 0) {
	    assert(cur->scheme_type == PGenerate::GS_CASE_ITEM
		   || pform_cur_generate->scheme_type != PGenerate::GS_CASE);
	    pform_cur_generate->generate_schemes.push_back(cur);
      } else {
	    assert(cur->scheme_type != PGenerate::GS_CASE_ITEM);
	    pform_cur_module->generate_schemes.push_back(cur);
      }
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
	    cerr << res->get_fileline() << ": warning: choosing ";
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

template <> inline svector<perm_string>::svector(unsigned size)
: nitems_(size), items_(new perm_string[size])
{
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

void pform_make_udp(perm_string name, list<perm_string>*parms,
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
      map<perm_string,PWire*> defs;
      for (unsigned idx = 0 ;  idx < decl->count() ;  idx += 1) {

	    perm_string port_name = (*decl)[idx]->basename();

	    if (PWire*cur = defs[port_name]) {
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
		  defs[port_name] = (*decl)[idx];
	    }
      }


	/* Put the parameters into a vector of wire descriptions. Look
	   in the map for the definitions of the name. In this loop,
	   the parms list in the list of ports in the port list of the
	   UDP declaration, and the defs map maps that name to a
	   PWire* created by an input or output declaration. */
      svector<PWire*> pins (parms->size());
      svector<perm_string> pin_names (parms->size());
      { list<perm_string>::iterator cur;
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
		  udp->ports[idx] = pins[idx]->basename();

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
      pins[0] = new PWire(out_name,
			  synchronous_flag? NetNet::REG : NetNet::WIRE,
			  NetNet::POUTPUT, IVL_VT_LOGIC);
      FILE_NAME(pins[0], file, lineno);

	/* Make the PWire objects for the input ports. */
      { list<perm_string>::iterator cur;
        unsigned idx;
        for (cur = parms->begin(), idx = 1
		   ;  cur != parms->end()
		   ;  idx += 1, cur++) {
	      assert(idx < pins.count());
	      pins[idx] = new PWire(*cur, NetNet::WIRE,
				    NetNet::PINPUT, IVL_VT_LOGIC);
	      FILE_NAME(pins[idx], file, lineno);
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
		  udp->ports[idx] = pins[idx]->basename();

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
static void pform_set_net_range(perm_string name,
				const svector<PExpr*>*range,
				bool signed_flag,
				ivl_variable_type_t dt,
				PWSRType rt,
				svector<named_pexpr_t*>*attr)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    VLerror("error: name is not a valid net.");
	    return;
      }

      if (range == 0) {
	      /* This is the special case that we really mean a
		 scalar. Set a fake range. */
	    cur->set_range(0, 0, rt, true);

      } else {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    cur->set_range((*range)[0], (*range)[1], rt, false);
      }
      cur->set_signed(signed_flag);

      if (dt != IVL_VT_NO_TYPE)
	    cur->set_data_type(dt);

      if (attr) {
	    for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		  named_pexpr_t*tmp = (*attr)[idx];
		  cur->attributes[tmp->name] = tmp->parm;
	    }
      }
}

void pform_set_net_range(list<perm_string>*names,
			 svector<PExpr*>*range,
			 bool signed_flag,
			 ivl_variable_type_t dt,
			 PWSRType rt)
{
      assert((range == 0) || (range->count() == 2));

      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_set_net_range(txt, range, signed_flag, dt, rt, 0);
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
      LexicalScope*scope = pform_get_cur_scope();

	// Check if the named event is already in the dictionary.
      if (scope->events.find(name) != scope->events.end()) {
	    LineInfo tloc;
	    FILE_NAME(&tloc, fn, ln);
	    cerr << tloc.get_fileline() << ": error: duplicate definition "
	            "for named event '" << name << "' in '"
	         << pform_cur_module->mod_name() << "'." << endl;
	    error_count += 1;
      }

      PEvent*event = new PEvent(name);
      FILE_NAME(event, fn, ln);
      scope->events[name] = event;
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
      FILE_NAME(cur, info.file, info.lineno);

      if (pform_cur_generate)
	    pform_cur_generate->add_gate(cur);
      else
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
 *
 * The first pform_make_modgate handles the case of a module
 * instantiated with ports passed by position. The "wires" is an
 * ordered array of port expressions.
 *
 * The second pform_make_modgate handles the case of a module
 * instantiated with ports passed by name. The "bind" argument is the
 * ports matched with names.
 */
static void pform_make_modgate(perm_string type,
			       perm_string name,
			       struct parmvalue_t*overrides,
			       svector<PExpr*>*wires,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      PGModule*cur = new PGModule(type, name, wires);
      FILE_NAME(cur, fn, ln);
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

      if (pform_cur_generate)
	    pform_cur_generate->add_gate(cur);
      else
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
      FILE_NAME(cur, fn, ln);
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


      if (pform_cur_generate)
	    pform_cur_generate->add_gate(cur);
      else
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

static PGAssign* pform_make_pgassign(PExpr*lval, PExpr*rval,
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

      if (pform_cur_generate)
	    pform_cur_generate->add_gate(cur);
      else
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
	      FILE_NAME(tmp, fn, lineno);
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
			perm_string name, PExpr*expr)
{
      if (! pform_at_module_level()) {
	    VLerror(li, "variable declaration assignments are only "
                        "allowed at the module level.");
	    delete expr;
	    return;
      }

      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    VLerror(li, "internal error: reginit to non-register?");
	    delete expr;
	    return;
      }

      PEIdent*lval = new PEIdent(name);
      FILE_NAME(lval, li);
      PAssign*ass = new PAssign(lval, expr, true);
      FILE_NAME(ass, li);
      PProcess*top = new PProcess(IVL_PR_INITIAL, ass);
      FILE_NAME(top, li);

      pform_put_behavior_in_scope(top);
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
			      perm_string name,
			      NetNet::PortType port_type,
			      NetNet::Type type,
			      bool signed_flag,
			      svector<PExpr*>*range,
			      svector<named_pexpr_t*>*attr)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur) {
	    ostringstream msg;
	    msg << name << " definition conflicts with "
		<< "definition at " << cur->get_fileline()
		<< ".";
	    VLerror(msg.str().c_str());
	    return;
      }


      cur = new PWire(name, type, port_type, IVL_VT_LOGIC);
      FILE_NAME(cur, li);

      cur->set_signed(signed_flag);

      if (range == 0) {
	    cur->set_range(0, 0, (type == NetNet::IMPLICIT) ? SR_PORT :
	                                                      SR_BOTH,
	                   true);

      } else {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    cur->set_range((*range)[0], (*range)[1],
	                   (type == NetNet::IMPLICIT) ? SR_PORT : SR_BOTH,
	                   false);
      }

      if (attr) {
	      for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		      named_pexpr_t*tmp = (*attr)[idx];
		      cur->attributes[tmp->name] = tmp->parm;
	      }
      }
      pform_put_wire_in_scope(name, cur);
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

/*
 * this is the basic form of pform_makewire. This takes a single simple
 * name, port type, net type, data type, and attributes, and creates
 * the variable/net. Other forms of pform_makewire ultimately call
 * this one to create the wire and stash it.
 */
void pform_makewire(const vlltype&li, perm_string name,
		    NetNet::Type type, NetNet::PortType pt,
		    ivl_variable_type_t dt,
		    svector<named_pexpr_t*>*attr)
{
      PWire*cur = pform_get_wire_in_scope(name);

	// If this is not implicit ("implicit" meaning we don't know
	// what the type is yet) then set the type now.
      if (cur && type != NetNet::IMPLICIT) {
	    bool rc = cur->set_wire_type(type);
	    if (rc == false) {
		  ostringstream msg;
		  msg << name << " " << type
		      << " definition conflicts with " << cur->get_wire_type()
		      << " definition at " << cur->get_fileline()
		      << ".";
		  VLerror(msg.str().c_str());
	    }

      }

      bool new_wire_flag = false;
      if (! cur) {
	    new_wire_flag = true;
	    cur = new PWire(name, type, pt, dt);
	    FILE_NAME(cur, li.text, li.first_line);
      }

      if (type != NetNet::IMPLICIT)
	    FILE_NAME(cur, li.text, li.first_line);

      bool flag;
      switch (dt) {
	  case IVL_VT_REAL:
	    flag = cur->set_data_type(dt);
	    if (flag == false) {
		  cerr << cur->get_fileline() << ": internal error: "
		       << " wire data type handling mismatch. Cannot change "
		       << cur->get_data_type()
		       << " to " << dt << "." << endl;
	    }
	    ivl_assert(*cur, flag);
	    cur->set_range(0, 0, SR_NET, true);
	    cur->set_signed(true);
	    break;
	  default:
	    break;
      }

      if (attr) {
	    for (unsigned idx = 0 ;  idx < attr->count() ;  idx += 1) {
		  named_pexpr_t*tmp = (*attr)[idx];
		  cur->attributes[tmp->name] = tmp->parm;
	    }
      }

      if (new_wire_flag)
	    pform_put_wire_in_scope(name, cur);
}

/*
 * This form takes a list of names and some type information, and
 * generates a bunch of variables/nets. We use the basic
 * pform_makewire above.
 */
void pform_makewire(const vlltype&li,
		    svector<PExpr*>*range,
		    bool signed_flag,
		    list<perm_string>*names,
		    NetNet::Type type,
		    NetNet::PortType pt,
		    ivl_variable_type_t dt,
		    svector<named_pexpr_t*>*attr,
		    PWSRType rt)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_makewire(li, txt, type, pt, dt, attr);
	    /* This has already been done for real variables. */
	    if (dt != IVL_VT_REAL) {
		  pform_set_net_range(txt, range, signed_flag, dt, rt, 0);
	    }
      }

      delete names;
      if (range)
	    delete range;
      if (attr)
	    delete attr;
}

/*
 * This form makes nets with delays and continuous assignments.
 */
void pform_makewire(const vlltype&li,
		    svector<PExpr*>*range,
		    bool signed_flag,
		    svector<PExpr*>*delay,
		    str_pair_t str,
		    net_decl_assign_t*decls,
		    NetNet::Type type,
		    ivl_variable_type_t dt)
{
      net_decl_assign_t*first = decls->next;
      decls->next = 0;

      while (first) {
	    net_decl_assign_t*next = first->next;

	    pform_makewire(li, first->name, type, NetNet::NOT_A_PORT, dt, 0);
	    /* This has already been done for real variables. */
	    if (dt != IVL_VT_REAL) {
		  pform_set_net_range(first->name, range, signed_flag, dt,
		                      SR_NET, 0);
	    }

	    PWire*cur = pform_get_wire_in_scope(first->name);
	    if (cur != 0) {
		  PEIdent*lval = new PEIdent(first->name);
		  FILE_NAME(lval, li.text, li.first_line);
		  PGAssign*ass = pform_make_pgassign(lval, first->expr,
						     delay, str);
		  FILE_NAME(ass, li.text, li.first_line);
	    }

	    delete first;
	    first = next;
      }
}

void pform_set_port_type(perm_string name, NetNet::PortType pt,
			 const char*file, unsigned lineno)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::IMPLICIT, NetNet::PIMPLICIT, IVL_VT_NO_TYPE);
	    FILE_NAME(cur, file, lineno);
	    pform_put_wire_in_scope(name, cur);
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
				      ivl_variable_type_t vtype,
				      bool signed_flag,
				      svector<PExpr*>*range,
				      list<perm_string>*names,
				      const char* file,
				      unsigned lineno,
				      bool isint)
{
      assert(names);
      svector<PWire*>*res = new svector<PWire*>(0);
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; cur ++ ) {

	    perm_string name = *cur;

	      /* Look for a preexisting wire. If it exists, set the
		 port direction. If not, create it. */
	    PWire*curw = pform_get_wire_in_scope(name);
	    if (curw) {
		  curw->set_port_type(pt);
	    } else {
		  curw = new PWire(name, NetNet::IMPLICIT_REG, pt, vtype);
		  FILE_NAME(curw, file, lineno);
		  pform_put_wire_in_scope(name, curw);
	    }

	    curw->set_signed(signed_flag);
	    if (isint) assert(curw->set_wire_type(NetNet::INTEGER));

	      /* If there is a range involved, it needs to be set. */
	    if (range)
		  curw->set_range((*range)[0], (*range)[1], SR_PORT, false);

	    svector<PWire*>*tmp = new svector<PWire*>(*res, curw);

	    delete res;
	    res = tmp;
      }

      if (range)
	    delete range;
      delete names;
      return res;
}

void pform_set_attrib(perm_string name, perm_string key, char*value)
{
      if (PWire*cur = lexical_scope->wires_find(name)) {
	    cur->attributes[key] = new PEString(value);

      } else if (PGate*curg = pform_cur_module->get_gate(name)) {
	    curg->attributes[key] = new PEString(value);

      } else {
	    delete[] value;
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
	    delete[] value;
	    return;
      }

      (*udp).second ->attributes[key] = new PEString(value);
}

/*
 * This function attaches a memory index range to an existing
 * register. (The named wire must be a register.
 */
void pform_set_reg_idx(perm_string name, PExpr*l, PExpr*r)
{
      PWire*cur = 0;
      if (pform_cur_generate) {
	    cur = pform_cur_generate->wires_find(name);
      } else {
	    cur = lexical_scope->wires_find(name);
      }
      if (cur == 0) {
	    VLerror("internal error: name is not a valid memory for index.");
	    return;
      }

      cur->set_memory_idx(l, r);
}

LexicalScope::range_t* pform_parameter_value_range(bool exclude_flag,
					     bool low_open, PExpr*low_expr,
					     bool hig_open, PExpr*hig_expr)
{
	// Detect +-inf and make the the *_open flags false to force
	// the range interpretation as inf.
      if (low_expr == 0) low_open = false;
      if (hig_expr == 0) hig_open = false;

      LexicalScope::range_t*tmp = new LexicalScope::range_t;
      tmp->exclude_flag = exclude_flag;
      tmp->low_open_flag = low_open;
      tmp->low_expr = low_expr;
      tmp->high_open_flag = hig_open;
      tmp->high_expr = hig_expr;
      tmp->next = 0;
      return tmp;
}

void pform_set_parameter(const struct vlltype&loc,
			 perm_string name, ivl_variable_type_t type,
			 bool signed_flag, svector<PExpr*>*range, PExpr*expr,
			 LexicalScope::range_t*value_range)
{
      LexicalScope*scope = pform_get_cur_scope();
      if (scope == pform_cur_generate) {
            VLerror("parameter declarations are not permitted in generate blocks");
            return;
      }

	// Check if the parameter name is already in the dictionary.
      if (scope->parameters.find(name) != scope->parameters.end()) {
	    LineInfo tloc;
	    FILE_NAME(&tloc, loc);
	    cerr << tloc.get_fileline() << ": error: duplicate definition "
	            "for parameter '" << name << "' in '"
	         << pform_cur_module->mod_name() << "'." << endl;
	    error_count += 1;
      }
      if (scope->localparams.find(name) != scope->localparams.end()) {
	    LineInfo tloc;
	    FILE_NAME(&tloc, loc);
	    cerr << tloc.get_fileline() << ": error: localparam and "
	            "parameter in '" << pform_cur_module->mod_name()
	         << "' have the same name '" << name << "'." << endl;
	    error_count += 1;
      }

      assert(expr);
      Module::param_expr_t&parm = scope->parameters[name];
      FILE_NAME(&parm, loc);

      parm.expr = expr;

      parm.type = type;
      if (range) {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    parm.msb = (*range)[0];
	    parm.lsb = (*range)[1];
      } else {
	    parm.msb = 0;
	    parm.lsb = 0;
      }
      parm.signed_flag = signed_flag;
      parm.range = value_range;

      if (scope == pform_cur_module)
            pform_cur_module->param_names.push_back(name);
}

void pform_set_localparam(const struct vlltype&loc,
			  perm_string name, ivl_variable_type_t type,
			  bool signed_flag, svector<PExpr*>*range, PExpr*expr)
{
      LexicalScope*scope = pform_get_cur_scope();

	// Check if the localparam name is already in the dictionary.
      if (scope->localparams.find(name) != scope->localparams.end()) {
	    LineInfo tloc;
	    FILE_NAME(&tloc, loc);
	    cerr << tloc.get_fileline() << ": error: duplicate definition "
	            "for localparam '" << name << "' in '"
	         << pform_cur_module->mod_name() << "'." << endl;
	    error_count += 1;
      }
      if (scope->parameters.find(name) != scope->parameters.end()) {
	    LineInfo tloc;
	    FILE_NAME(&tloc, loc);
	    cerr << tloc.get_fileline() << ": error: parameter and "
	            "localparam in '" << pform_cur_module->mod_name()
	         << "' have the same name '" << name << "'." << endl;
	    error_count += 1;
      }

      assert(expr);
      Module::param_expr_t&parm = pform_get_cur_scope()->localparams[name];
      FILE_NAME(&parm, loc);

      parm.expr = expr;

      parm.type = type;
      if (range) {
	    assert(range->count() == 2);
	    assert((*range)[0]);
	    assert((*range)[1]);
	    parm.msb = (*range)[0];
	    parm.lsb = (*range)[1];
      } else {
	    parm.msb  = 0;
	    parm.lsb  = 0;
      }
      parm.signed_flag = signed_flag;
      parm.range = 0;
}

void pform_set_specparam(perm_string name, PExpr*expr)
{
	// Check if the specparam name is already in the dictionary.
      if (pform_cur_module->specparams.find(name) !=
          pform_cur_module->specparams.end()) {
	    cerr << expr->get_fileline() << ": error: duplicate definition "
	            "for specparam '" << name << "' in '"
	         << pform_cur_module->mod_name() << "'." << endl;
	    error_count += 1;
      }

      assert(expr);
      pform_cur_module->specparams[name] = expr;
}

void pform_set_defparam(const pform_name_t&name, PExpr*expr)
{
      assert(expr);
      if (pform_cur_generate)
            pform_cur_generate->defparms.push_back(make_pair(name,expr));
      else
            pform_cur_module->defparms.push_back(make_pair(name,expr));
}

/*
 * Specify paths.
 */
extern PSpecPath* pform_make_specify_path(const struct vlltype&li,
					  list<perm_string>*src, char pol,
					  bool full_flag, list<perm_string>*dst)
{
      PSpecPath*path = new PSpecPath(src->size(), dst->size());
      FILE_NAME(path, li.text, li.first_line);
      path->polarity = pol;
      path->full_flag = full_flag;

      unsigned idx;
      list<perm_string>::const_iterator cur;

      idx = 0;
      for (idx = 0, cur = src->begin() ;  cur != src->end() ;  idx++, cur++) {
	    path->src[idx] = *cur;
      }
      assert(idx == path->src.size());
      delete src;

      for (idx = 0, cur = dst->begin() ;  cur != dst->end() ;  idx++, cur++) {
	    path->dst[idx] = *cur;
      }
      assert(idx == path->dst.size());
      delete dst;

      return path;
}

extern PSpecPath*pform_make_specify_edge_path(const struct vlltype&li,
					 int edge_flag, /*posedge==true */
					 list<perm_string>*src, char pol,
					 bool full_flag, list<perm_string>*dst,
					 PExpr*data_source_expression)
{
      PSpecPath*tmp = pform_make_specify_path(li, src, pol, full_flag, dst);
      tmp->edge = edge_flag;
      tmp->data_source_expression = data_source_expression;
      return tmp;
}

extern PSpecPath* pform_assign_path_delay(PSpecPath*path, svector<PExpr*>*del)
{
      if (path == 0)
	    return 0;

      assert(path->delays.size() == 0);

      path->delays.resize(del->count());
      for (unsigned idx = 0 ;  idx < path->delays.size() ;  idx += 1)
	    path->delays[idx] = (*del)[idx];

      delete del;

      return path;
}


extern void pform_module_specify_path(PSpecPath*obj)
{
      if (obj == 0)
	    return;
      pform_cur_module->specify_paths.push_back(obj);
}

void pform_set_port_type(const struct vlltype&li,
			 list<perm_string>*names,
			 svector<PExpr*>*range,
			 bool signed_flag,
			 NetNet::PortType pt,
			 svector<named_pexpr_t*>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end()
		 ; cur ++ ) {
	    perm_string txt = *cur;
	    pform_set_port_type(txt, pt, li.text, li.first_line);
	    pform_set_net_range(txt, range, signed_flag, IVL_VT_NO_TYPE,
	                        SR_PORT, attr);
      }

      delete names;
      if (range)
	    delete range;
      if (attr)
	    delete attr;
}

static void pform_set_reg_integer(perm_string name)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::INTEGER,
			    NetNet::NOT_A_PORT,
			    IVL_VT_LOGIC);
	    cur->set_signed(true);
	    pform_put_wire_in_scope(name, cur);
      } else {
	    bool rc = cur->set_wire_type(NetNet::INTEGER);
	    assert(rc);
	    cur->set_data_type(IVL_VT_LOGIC);
	    cur->set_signed(true);
      }
      assert(cur);

      cur->set_range(new PENumber(new verinum(integer_width-1, integer_width)),
		     new PENumber(new verinum((uint64_t)0, integer_width)),
		     SR_NET, false);
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

static void pform_set_reg_time(perm_string name)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    cur = new PWire(name, NetNet::REG, NetNet::NOT_A_PORT, IVL_VT_LOGIC);
	    pform_put_wire_in_scope(name, cur);
      } else {
	    bool rc = cur->set_wire_type(NetNet::REG);
	    assert(rc);
	    rc = cur->set_data_type(IVL_VT_LOGIC);
	    assert(rc);
      }
      assert(cur);

      cur->set_range(new PENumber(new verinum(TIME_WIDTH-1, integer_width)),
		     new PENumber(new verinum((uint64_t)0, integer_width)),
		     SR_NET, false);
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
	    PWire*pp = new PWire(txt,
				 NetNet::IMPLICIT,
				 NetNet::PINPUT,
				 IVL_VT_LOGIC);
	    (*out)[idx] = pp;
	    idx += 1;
      }

      delete names;
      return out;
}

PProcess* pform_make_behavior(ivl_process_type_t type, Statement*st,
			      svector<named_pexpr_t*>*attr)
{
      PProcess*pp = new PProcess(type, st);

      pform_bind_attributes(pp->attributes, attr);

      pform_put_behavior_in_scope(pp);
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

      destroy_lexor();
      return error_count;
}

void pform_error_nested_modules()
{
      assert( pform_cur_module != 0 );
      cerr << pform_cur_module->get_fileline() << ": error: original module "
              "(" << pform_cur_module->mod_name() << ") defined here." << endl;
}
