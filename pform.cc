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
# include  "PClass.h"
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
# include  "ivl_alloc.h"

map<perm_string,Module*> pform_modules;
map<perm_string,PUdp*> pform_primitives;


/*
 * Parse configuration file with format <key>=<value>, where key
 * is the hierarchical name of a valid parameter name, and value
 * is the value user wants to assign to. The value should be constant.
 */
void parm_to_defparam_list(const string&param)
{
    char* key;
    char* value;
    unsigned off = param.find('=');
    if (off > param.size()) {
        key = strdup(param.c_str());
        value = (char*)malloc(1);
        *value = '\0';

    } else {
        key = strdup(param.substr(0, off).c_str());
        value = strdup(param.substr(off+1).c_str());
    }

    // Resolve hierarchical name for defparam. Remember
    // to deal with bit select for generate scopes. Bit
    // select expression should be constant integer.
    pform_name_t name;
    char *nkey = key;
    char *ptr = strchr(key, '.');
    while (ptr != 0) {
        *ptr++ = '\0';
        // Find if bit select is applied, this would be something
        // like - scope[2].param = 10
        char *bit_l = strchr(nkey, '[');
        if (bit_l !=0) {
            *bit_l++ = '\0';
            char *bit_r = strchr(bit_l, ']');
            if (bit_r == 0) {
                cerr << "<command line>: error: missing ']' for defparam: " << nkey << endl;
                free(key);
                free(value);
                return;
            }
            *bit_r = '\0';
            int i = 0;
            while (*(bit_l+i) != '\0')
                if (!isdigit(*(bit_l+i++))) {
                    cerr << "<command line>: error: scope index expression is not constant: " << nkey << endl;
                    free(key);
                    free(value);
                    return;
                }
            name_component_t tmp(lex_strings.make(nkey));
            index_component_t index;
            index.sel = index_component_t::SEL_BIT;
            verinum *seln = new verinum(atoi(bit_l));
            PENumber *sel = new PENumber(seln);
            index.msb = sel;
            index.lsb = sel;
            tmp.index.push_back(index);
            name.push_back(tmp);
        }
        else    // no bit select
            name.push_back(name_component_t(lex_strings.make(nkey)));

        nkey = ptr;
        ptr = strchr(nkey, '.');
    }
    name.push_back(name_component_t(lex_strings.make(nkey)));

    // Resolve value to PExpr class. Should support all kind of constant
    // format including based number, dec number, real number and string.
    if (*value == '"') {    // string type
        char *buf = strdup (value);
        char *buf_ptr = buf+1;
        // Parse until another '"' or '\0'
        while (*buf_ptr != '"' && *buf_ptr != '\0') {
            buf_ptr++;
            // Check for escape, especially '\"', which does not mean the
            // end of string.
            if (*buf_ptr == '\\' && *(buf_ptr+1) != '\0')
                buf_ptr += 2;
        }
        if (*buf_ptr == '\0')   // String end without '"'
            cerr << "<command line>: error: missing close quote of string for defparam: " << name << endl;
        else if (*(buf_ptr+1) != 0) { // '"' appears within string with no escape
            cerr << buf_ptr << endl;
            cerr << "<command line>: error: \'\"\' appears within string value for defparam: " << name
                 << ". Ignore characters after \'\"\'" << endl;
        }

        *buf_ptr = '\0';
        buf_ptr = buf+1;
        // Remember to use 'new' to allocate string for PEString
        // because 'delete' is used by its destructor.
        char *nchar = strcpy(new char [strlen(buf_ptr)+1], buf_ptr);
        PExpr* ndec = new PEString(nchar);
	Module::user_defparms.push_back( make_pair(name, ndec) );
        free(buf);
    }
    else {      // number type
        char *num = strchr(value, '\'');
        if (num != 0) {
            verinum *val;
            // BASED_NUMBER, something like - scope.parameter='b11
            // make sure to check 'h' first because 'b'&'d' may be included
            // in hex format
            if (strchr(num, 'h') || strchr(num, 'H'))
                val = make_unsized_hex(num);
            else if (strchr(num, 'd') || strchr(num, 'D'))
                if (strchr(num, 'x') || strchr(num, 'X') || strchr(num, 'z') || strchr(num, 'Z'))
                    val = make_undef_highz_dec(num);
                else
                    val = make_unsized_dec(num);
            else if (strchr(num, 'b') || strchr(num, 'B')) {
                val = make_unsized_binary(num);
            }
            else if (strchr(num, 'o') || strchr(num, 'O'))
                val = make_unsized_octal(num);
            else {
                cerr << "<command line>: error: value specify error for defparam: " << name << endl;
                free(key);
                free(value);
                return;
            }

            // BASED_NUMBER with size, something like - scope.parameter=2'b11
            if (num != value) {
                *num = 0;
                verinum *siz = make_unsized_dec(value);
                val = pform_verinum_with_size(siz, val, "<command line>", 0);
            }

            PExpr* ndec = new PENumber(val);
	    Module::user_defparms.push_back( make_pair(name, ndec) );

        }
        else {
            // REALTIME, something like - scope.parameter=1.22 or scope.parameter=1e2
            if (strchr(value, '.') || strchr(value, 'e') || strchr(value, 'E')) {
                verireal *val = new verireal(value);
                PExpr* nreal = new PEFNumber(val);
		Module::user_defparms.push_back( make_pair(name, nreal) );
            }
            else {
                // DEC_NUMBER, something like - scope.parameter=3
                verinum *val = make_unsized_dec(value);
                PExpr* ndec = new PENumber(val);
		Module::user_defparms.push_back( make_pair(name, ndec) );
            }
        }
    }
    free(key);
    free(value);
}

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

/* These two flags check the initial timeprecision and timeunit
 * declaration inside a module.
 */
static bool tp_decl_flag = false;
static bool tu_decl_flag = false;

/*
 * Flags used to set time_from_timescale based on timeunit and
 * timeprecision.
 */
static bool tu_global_flag = false;
static bool tp_global_flag = false;
static bool tu_local_flag = false;
static bool tp_local_flag = false;

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
static LexicalScope* lexical_scope = 0;

void pform_pop_scope()
{
      assert(lexical_scope);
      lexical_scope = lexical_scope->parent_scope();
}

PClass* pform_push_class_scope(const struct vlltype&loc, perm_string name)
{
      PClass*class_scope = new PClass(name, lexical_scope);
      FILE_NAME(class_scope, loc);

      lexical_scope = class_scope;
      return class_scope;
}

PTask* pform_push_task_scope(const struct vlltype&loc, char*name, bool is_auto)
{
      perm_string task_name = lex_strings.make(name);

      PTask*task = new PTask(task_name, lexical_scope, is_auto);
      FILE_NAME(task, loc);

      LexicalScope*scope = lexical_scope;
      PScopeExtra*scopex = dynamic_cast<PScopeExtra*> (scope);
      while (scope && !scopex) {
	    scope = scope->parent_scope();
	    scopex = dynamic_cast<PScopeExtra*> (scope);
      }
      assert(scopex);

      if (pform_cur_generate) {
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
      } else {
	      // Check if the task is already in the dictionary.
	    if (scopex->tasks.find(task->pscope_name()) != scopex->tasks.end()) {
		  cerr << task->get_fileline() << ": error: duplicate "
		          "definition for task '" << name << "' in '"
		       << scopex->pscope_name() << "'." << endl;
		  error_count += 1;
	    }
	    scopex->tasks[task->pscope_name()] = task;
      }

      lexical_scope = task;

      return task;
}

PFunction* pform_push_function_scope(const struct vlltype&loc, char*name,
                                      bool is_auto)
{
      perm_string func_name = lex_strings.make(name);

      PFunction*func = new PFunction(func_name, lexical_scope, is_auto);
      FILE_NAME(func, loc);

      LexicalScope*scope = lexical_scope;
      PScopeExtra*scopex = dynamic_cast<PScopeExtra*> (scope);
      while (scope && !scopex) {
	    scope = scope->parent_scope();
	    scopex = dynamic_cast<PScopeExtra*> (scope);
      }
      assert(scopex);

      if (pform_cur_generate) {
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
      } else {
	      // Check if the function is already in the dictionary.
	    if (scopex->funcs.find(func->pscope_name()) != scopex->funcs.end()) {
		  cerr << func->get_fileline() << ": error: duplicate "
		          "definition for function '" << name << "' in '"
		       << scopex->pscope_name() << "'." << endl;
		  error_count += 1;
	    }
	    scopex->funcs[func->pscope_name()] = func;
      }
      lexical_scope = func;

      return func;
}

PBlock* pform_push_block_scope(char*name, PBlock::BL_TYPE bt)
{
      perm_string block_name = lex_strings.make(name);

      PBlock*block = new PBlock(block_name, lexical_scope, bt);
      lexical_scope = block;

      return block;
}

PGenerate* pform_parent_generate(void)
{
      return pform_cur_generate;
}

void pform_bind_attributes(map<perm_string,PExpr*>&attributes,
			   list<named_pexpr_t>*attr, bool keep_attrs)
{
      if (attr == 0)
	    return;

      while (! attr->empty()) {
	    named_pexpr_t tmp = attr->front();
	    attr->pop_front();
	    attributes[tmp.name] = tmp.parm;
      }
      if (!keep_attrs)
	    delete attr;
}

static bool pform_at_module_level()
{
      return (lexical_scope == pform_cur_module)
          || (lexical_scope == pform_cur_generate);
}

PWire*pform_get_wire_in_scope(perm_string name)
{
      return lexical_scope->wires_find(name);
}

static void pform_put_wire_in_scope(perm_string name, PWire*net)
{
      lexical_scope->wires[name] = net;
}

static void pform_put_enum_type_in_scope(enum_type_t*enum_set)
{
      lexical_scope->enum_sets.push_back(enum_set);
}

PWire*pform_get_make_wire_in_scope(perm_string name, NetNet::Type net_type, NetNet::PortType port_type, ivl_variable_type_t vt_type)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    cur = new PWire(name, net_type, port_type, vt_type);
	    pform_put_wire_in_scope(name, cur);
      } else {
	    bool rc = cur->set_wire_type(net_type);
	    assert(rc);
	    rc = cur->set_data_type(vt_type);
	    assert(rc);
      }

      return cur;
}

void pform_set_typedef(perm_string name, data_type_t*data_type)
{
      data_type_t*&ref = lexical_scope->typedefs[name];
      ivl_assert(*data_type, ref == 0);
      ref = data_type;
}

data_type_t* pform_test_type_identifier(const char*txt)
{
	// If there is no lexical_scope yet, then there is NO WAY the
	// identifier can be a type_identifier.
      if (lexical_scope == 0)
	    return 0;

      perm_string name = lex_strings.make(txt);
      map<perm_string,data_type_t*>::iterator cur;
      LexicalScope*cur_scope = lexical_scope;
      do {
	    cur = cur_scope->typedefs.find(name);
	    if (cur != cur_scope->typedefs.end())
		  return cur->second;
 
	    cur_scope = cur_scope->parent_scope();
      } while (cur_scope);
      return 0;
}

static void pform_put_behavior_in_scope(PProcess*pp)
{
      lexical_scope->behaviors.push_back(pp);
}

void pform_put_behavior_in_scope(AProcess*pp)
{
      lexical_scope->analog_behaviors.push_back(pp);
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

static void pform_declare_implicit_nets(PExpr*expr)
{
	/* If implicit net creation is turned off, then stop now. */
      if (pform_default_nettype == NetNet::NONE)
	    return;

      if (expr)
            expr->declare_implicit_nets(lexical_scope, pform_default_nettype);
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
	/* A `timescale clears the timeunit/timeprecision state. */
      tu_global_flag = false;
      tp_global_flag = false;

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
      for (mod = pform_modules.begin(); mod != pform_modules.end(); ++ mod ) {
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
		       ; mod != pform_modules.end() ; ++ mod ) {
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

bool get_time_unit(const char*cp, int &unit)
{
	const char *c;
	bool        rc = true;

	if (strchr(cp, '_')) {
		VLerror(yylloc, "Invalid timeunit constant ('_' is not "
				"supported).");
		return false;
	}

	c = strpbrk(cp, "munpfs");
	if (!c)
		return false;

	if (*c == 's')
		unit = 0;
	else if (!strncmp(c, "ms", 2))
		unit = -3;
	else if (!strncmp(c, "us", 2))
		unit = -6;
	else if (!strncmp(c, "ns", 2))
		unit = -9;
	else if (!strncmp(c, "ps", 2))
		unit = -12;
	else if (!strncmp(c, "fs", 2))
		unit = -15;
	else {
		rc = false;

		ostringstream msg;
		msg << "Invalid timeunit scale '" << cp << "'.";
		VLerror(msg.str().c_str());
	}

	return rc;
}

/*
 * Get a timeunit or timeprecision value from a string.  This is
 * similar to the code in lexor.lex for the `timescale directive.
 */
static bool get_time_unit_prec(const char*cp, int &res, bool is_unit)
{
	/* We do not support a '_' in these time constants. */
      if (strchr(cp, '_')) {
	    if (is_unit) {
		  VLerror(yylloc, "Invalid timeunit constant ('_' is not "
		                  "supported).");
	    } else {
		  VLerror(yylloc, "Invalid timeprecision constant ('_' is not "
		                  "supported).");
	    }
	    return true;
      }

	/* Check for the 1 digit. */
      if (*cp != '1') {
	    if (is_unit) {
		  VLerror(yylloc, "Invalid timeunit constant (1st digit).");
	    } else {
		  VLerror(yylloc, "Invalid timeprecision constant (1st digit).");
	    }
	    return true;
      }
      cp += 1;

	/* Check the number of zeros after the 1. */
      res = strspn(cp, "0");
      if (res > 2) {
	    if (is_unit) {
		  VLerror(yylloc, "Invalid timeunit constant (number of "
		                  "zeros).");
	    } else {
		  VLerror(yylloc, "Invalid timeprecision constant (number of "
		                  "zeros).");
	    }
	    return true;
      }
      cp += res;

	/* Now process the scaling string. */
      if (strncmp("s", cp, 1) == 0) {
	    res -= 0;
	    return false;

      } else if (strncmp("ms", cp, 2) == 0) {
	    res -= 3;
	    return false;

      } else if (strncmp("us", cp, 2) == 0) {
	    res -= 6;
	    return false;

      } else if (strncmp("ns", cp, 2) == 0) {
	    res -= 9;
	    return false;

      } else if (strncmp("ps", cp, 2) == 0) {
	    res -= 12;
	    return false;

      } else if (strncmp("fs", cp, 2) == 0) {
	    res -= 15;
	    return false;

      }

      ostringstream msg;
      msg << "Invalid ";
      if (is_unit) msg << "timeunit";
      else msg << "timeprecision";
      msg << " scale '" << cp << "'.";
      VLerror(msg.str().c_str());
      return true;
}

void pform_set_timeunit(const char*txt, bool in_module, bool only_check)
{
      int val;

      if (get_time_unit_prec(txt, val, true)) return;

      if (in_module) {
	    if (!only_check) {
		  pform_cur_module->time_unit = val;
		  tu_decl_flag = true;
		  tu_local_flag = true;
	    } else if (!tu_decl_flag) {
		  VLerror(yylloc, "error: repeat timeunit found and the "
		                  "initial module timeunit is missing.");
		  return;
	    } else if (pform_cur_module->time_unit != val) {
		  VLerror(yylloc, "error: repeat timeunit does not match "
		                  "the initial module timeunit "
		                  "declaration.");
		  return;
	    }

      } else {
	      /* Skip a global timeunit when `timescale is defined. */
	    if (pform_timescale_file) return;
	    tu_global_flag = true;
	    pform_time_unit = val;
      }
}

int pform_get_timeunit()
{
	return pform_cur_module->time_unit;
}

void pform_set_timeprecision(const char*txt, bool in_module, bool only_check)
{
      int val;

      if (get_time_unit_prec(txt, val, false)) return;

      if (in_module) {
	    if (!only_check) {
		  pform_cur_module->time_precision = val;
		  tp_decl_flag = true;
		  tp_local_flag = true;
	    } else if (!tp_decl_flag) {
		  VLerror(yylloc, "error: repeat timeprecision found and the "
		                  "initial module timeprecision is missing.");
		  return;
	    } else if (pform_cur_module->time_precision != val) {
		  VLerror(yylloc, "error: repeat timeprecision does not match "
		                  "the initial module timeprecision "
		                  "declaration.");
		  return;
	    }
      } else {
	      /* Skip a global timeprecision when `timescale is defined. */
	    if (pform_timescale_file) return;
	    pform_time_prec = val;
	    tp_global_flag=true;
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
		       list<named_pexpr_t>*attr)
{
      assert( pform_cur_module == 0 );

      perm_string lex_name = lex_strings.make(name);
      pform_cur_module = new Module(lex_name);
	/* Set the local time unit/precision to the global value. */
      pform_cur_module->time_unit = pform_time_unit;
      pform_cur_module->time_precision = pform_time_prec;
      tu_local_flag = tu_global_flag;
      tp_local_flag = tp_global_flag;

	/* If we have a timescale file then the time information is from
	 * a timescale directive. */
      pform_cur_module->time_from_timescale = pform_timescale_file != 0;

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
      pform_bind_attributes(pform_cur_module->attributes, attr);
}

/*
 * In SystemVerilog we can have separate timeunit and timeprecision
 * declarations. We need to have the values worked out by time this
 * task is called.
 */
void pform_check_timeunit_prec()
{
      assert(pform_cur_module);
      if ((generation_flag & (GN_VER2005_SV | GN_VER2009)) &&
          (pform_cur_module->time_unit < pform_cur_module->time_precision)) {
	    VLerror("error: a timeprecision is missing or is too "
	            "large!");
      } else assert(pform_cur_module->time_unit >=
                    pform_cur_module->time_precision);
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
      pform_cur_module->time_from_timescale = (tu_local_flag &&
                                               tp_local_flag) ||
                                              (pform_timescale_file != 0);
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
      pform_pop_scope();
      ivl_assert(*pform_cur_module, lexical_scope == 0);

      pform_cur_module = 0;
      tp_decl_flag = false;
      tu_decl_flag = false;
      tu_local_flag = false;
      tp_local_flag = false;
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
      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
      lexical_scope = gen;

      FILE_NAME(gen, li);

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
      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
      lexical_scope = gen;

      FILE_NAME(gen, li);

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

      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
      lexical_scope = gen;

      FILE_NAME(gen, li);

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
      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
      lexical_scope = gen;

      FILE_NAME(gen, li);

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
      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
      lexical_scope = gen;

      FILE_NAME(gen, li);

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
void pform_generate_case_item(const struct vlltype&li, list<PExpr*>*expr_list)
{
      assert(pform_cur_generate);
      assert(pform_cur_generate->scheme_type == PGenerate::GS_CASE);

      PGenerate*gen = new PGenerate(lexical_scope, pform_cur_generate->id_number);
      lexical_scope = gen;

      FILE_NAME(gen, li);

      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_CASE_ITEM;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = 0;
      pform_cur_generate->loop_step = 0;

      if (expr_list != 0) {
	    list<PExpr*>::iterator expr_cur = expr_list->begin();
	    pform_cur_generate->item_test.resize(expr_list->size());
	    for (unsigned idx = 0 ; idx < expr_list->size() ; idx += 1) {
		  pform_cur_generate->item_test[idx] = *expr_cur;
		  ++ expr_cur;
	    }
	    assert(expr_cur == expr_list->end());
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

	// The current lexical scope should be this generate construct by now
      ivl_assert(*pform_cur_generate, lexical_scope == pform_cur_generate);
      pform_pop_scope();

      PGenerate*parent_generate = dynamic_cast<PGenerate*>(lexical_scope);
      if (parent_generate) {
	    assert(pform_cur_generate->scheme_type == PGenerate::GS_CASE_ITEM
		   || parent_generate->scheme_type != PGenerate::GS_CASE);
	    parent_generate->generate_schemes.push_back(pform_cur_generate);
      } else {
	    assert(pform_cur_generate->scheme_type != PGenerate::GS_CASE_ITEM);
	    pform_cur_module->generate_schemes.push_back(pform_cur_generate);
      }
      pform_cur_generate = parent_generate;
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
		   ; cur != table->end() ; ++ cur , idx += 1) {
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
      assert(!parms->empty());

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
	    FILE_NAME(udp, file, lineno);

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
	    FILE_NAME(udp, file, lineno);

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

static void ranges_from_list(list<PWire::range_t>&rlist,
			     const list<index_component_t>*range)
{
	// Convert a list of index_component_t to PWire::range_t.
      for (list<index_component_t>::const_iterator rcur = range->begin()
		 ; rcur != range->end() ; ++rcur) {
	    PWire::range_t rng;
	    rng.msb = rcur->msb;
	    rng.lsb = rcur->lsb;
	    rlist.push_back(rng);
      }
}

/*
 * This function attaches a range to a given name. The function is
 * only called by the parser within the scope of the net declaration,
 * and the name that I receive only has the tail component.
 */
static void pform_set_net_range(perm_string name,
				const list<index_component_t>*range,
				bool signed_flag,
				ivl_variable_type_t dt,
				PWSRType rt)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    VLerror("error: name is not a valid net.");
	    return;
      }

      if (range == 0) {
	      /* This is the special case that we really mean a
		 scalar. Set a fake range. */
	    cur->set_range_scalar(rt);

      } else {
	    list<PWire::range_t> rlist;
	    ranges_from_list(rlist, range);
	    cur->set_range(rlist, rt);
      }
      cur->set_signed(signed_flag);

      if (dt != IVL_VT_NO_TYPE)
	    cur->set_data_type(dt);
}

static void pform_set_net_range(list<perm_string>*names,
				list<index_component_t>*range,
				bool signed_flag,
				ivl_variable_type_t dt)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_net_range(txt, range, signed_flag, dt, SR_NET);
      }

      delete names;
      delete range;
}

/*
 * This is invoked to make a named event. This is the declaration of
 * the event, and not necessarily the use of it.
 */
static void pform_make_event(perm_string name, const char*fn, unsigned ln)
{
	// Check if the named event is already in the dictionary.
      if (lexical_scope->events.find(name) != lexical_scope->events.end()) {
	    LineInfo tloc;
	    FILE_NAME(&tloc, fn, ln);
	    cerr << tloc.get_fileline() << ": error: duplicate definition "
	            "for named event '" << name << "' in '"
	         << pform_cur_module->mod_name() << "'." << endl;
	    error_count += 1;
      }

      PEvent*event = new PEvent(name);
      FILE_NAME(event, fn, ln);
      lexical_scope->events[name] = event;
}

void pform_make_events(list<perm_string>*names, const char*fn, unsigned ln)
{
      list<perm_string>::iterator cur;
      for (cur = names->begin() ;  cur != names->end() ; ++ cur ) {
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
static void pform_makegate(PGBuiltin::Type type,
			   struct str_pair_t str,
			   list<PExpr*>* delay,
			   const lgate&info,
			   list<named_pexpr_t>*attr)
{
      if (info.parms_by_name) {
	    cerr << info.file << ":" << info.lineno << ": Gates do not "
		  "have port names." << endl;
	    error_count += 1;
	    return;
      }

      for (list<PExpr*>::iterator cur = info.parms->begin()
		 ; cur != info.parms->end() ; ++cur) {
	    pform_declare_implicit_nets(*cur);
      }

      perm_string dev_name = lex_strings.make(info.name);
      PGBuiltin*cur = new PGBuiltin(type, dev_name, info.parms, delay);
      if (info.range.msb)
	    cur->set_range(info.range.msb, info.range.lsb);

	// The pform_makegates() that calls me will take care of
	// deleting the attr pointer, so tell the
	// pform_bind_attributes function to keep the attr object.
      pform_bind_attributes(cur->attributes, attr, true);

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
		     list<PExpr*>*delay,
		     svector<lgate>*gates,
		     list<named_pexpr_t>*attr)
{
      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    pform_makegate(type, str, delay, (*gates)[idx], attr);
      }

      if (attr) delete attr;
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
			       list<PExpr*>*wires,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      for (list<PExpr*>::iterator idx = wires->begin()
		 ; idx != wires->end() ; ++idx) {
	    pform_declare_implicit_nets(*idx);
      }

      PGModule*cur = new PGModule(type, name, wires);
      FILE_NAME(cur, fn, ln);
      cur->set_range(msb,lsb);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->size();
	    named<PExpr*>*byname = new named<PExpr*>[cnt];

	    list<named_pexpr_t>::iterator by_name_cur = overrides->by_name->begin();
	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1, ++ by_name_cur) {
		  byname[idx].name = by_name_cur->name;
		  byname[idx].parm = by_name_cur->parm;
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
			       list<named_pexpr_t>*bind,
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln)
{
      unsigned npins = bind->size();
      named<PExpr*>*pins = new named<PExpr*>[npins];
      list<named_pexpr_t>::iterator bind_cur = bind->begin();
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1,  ++bind_cur) {
	    pins[idx].name = bind_cur->name;
	    pins[idx].parm = bind_cur->parm;
            pform_declare_implicit_nets(bind_cur->parm);
      }

      PGModule*cur = new PGModule(type, name, pins, npins);
      FILE_NAME(cur, fn, ln);
      cur->set_range(msb,lsb);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->size();
	    named<PExpr*>*byname = new named<PExpr*>[cnt];

	    list<named_pexpr_t>::iterator by_name_cur = overrides->by_name->begin();
	    for (unsigned idx = 0 ;  idx < cnt ;  idx += 1,  ++by_name_cur) {
		  byname[idx].name = by_name_cur->name;
		  byname[idx].parm = by_name_cur->parm;
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
				     cur.range.msb, cur.range.lsb,
				     cur.file, cur.lineno);

	    } else if (cur.parms) {

		    /* If there are no parameters, the parser will be
		       tricked into thinking it is one empty
		       parameter. This fixes that. */
		  if ((cur.parms->size() == 1) && (cur.parms->front() == 0)) {
			delete cur.parms;
			cur.parms = new list<PExpr*>;
		  }
		  pform_make_modgate(type, cur_name, overrides,
				     cur.parms,
				     cur.range.msb, cur.range.lsb,
				     cur.file, cur.lineno);

	    } else {
		  list<PExpr*>*wires = new list<PExpr*>;
		  pform_make_modgate(type, cur_name, overrides,
				     wires,
				     cur.range.msb, cur.range.lsb,
				     cur.file, cur.lineno);
	    }
      }

      delete gates;
}

static PGAssign* pform_make_pgassign(PExpr*lval, PExpr*rval,
			      list<PExpr*>*del,
			      struct str_pair_t str)
{
        /* Implicit declaration of nets on the LHS of a continuous
           assignment was introduced in IEEE1364-2001. */
      if (generation_flag != GN_VER1995)
            pform_declare_implicit_nets(lval);

      list<PExpr*>*wires = new list<PExpr*>;
      wires->push_back(lval);
      wires->push_back(rval);

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

void pform_make_pgassign_list(list<PExpr*>*alist,
			      list<PExpr*>*del,
			      struct str_pair_t str,
			      const char* fn,
			      unsigned lineno)
{
      assert(alist->size() % 2 == 0);
      while (! alist->empty()) {
	    PExpr*lval = alist->front(); alist->pop_front();
	    PExpr*rval = alist->front(); alist->pop_front();
	    PGAssign*tmp = pform_make_pgassign(lval, rval, del, str);
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
      if (! pform_at_module_level() && !gn_system_verilog()) {
	    VLerror(li, "error: variable declaration assignments are only "
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
			      ivl_variable_type_t data_type,
			      bool signed_flag,
			      list<index_component_t>*range,
			      list<named_pexpr_t>*attr)
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

	// The default type for all flavor of ports is LOGIC.
      if (data_type == IVL_VT_NO_TYPE)
	    data_type = IVL_VT_LOGIC;

      cur = new PWire(name, type, port_type, data_type);
      FILE_NAME(cur, li);

      cur->set_signed(signed_flag);

      if (range == 0) {
	    cur->set_range_scalar((type == NetNet::IMPLICIT) ? SR_PORT : SR_BOTH);

      } else {
	    list<PWire::range_t> rlist;
	    ranges_from_list(rlist, range);
	    cur->set_range(rlist, (type == NetNet::IMPLICIT) ? SR_PORT : SR_BOTH);
      }

      pform_bind_attributes(cur->attributes, attr);
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

static PWire* pform_get_or_make_wire(const vlltype&li, perm_string name,
			      NetNet::Type type, NetNet::PortType ptype,
			      ivl_variable_type_t dtype)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur) {
	      // If this is not implicit ("implicit" meaning we don't
	      // know what the type is yet) then set the type now.
	    if (type != NetNet::IMPLICIT) {
		  bool rc = cur->set_wire_type(type);
		  if (rc == false) {
			ostringstream msg;
			msg << name << " " << type
			    << " definition conflicts with " << cur->get_wire_type()
			    << " definition at " << cur->get_fileline()
			    << ".";
			VLerror(msg.str().c_str());
		  }
		  FILE_NAME(cur, li.text, li.first_line);
	    }
	    return cur;
      }

      cur = new PWire(name, type, ptype, dtype);
      FILE_NAME(cur, li.text, li.first_line);

      pform_put_wire_in_scope(name, cur);
      return cur;
}

/*
 * this is the basic form of pform_makewire. This takes a single simple
 * name, port type, net type, data type, and attributes, and creates
 * the variable/net. Other forms of pform_makewire ultimately call
 * this one to create the wire and stash it.
 */
void pform_makewire(const vlltype&li, perm_string name,
		    NetNet::Type type, NetNet::PortType pt,
		    ivl_variable_type_t dt,
		    list<named_pexpr_t>*attr)
{
      PWire*cur = pform_get_or_make_wire(li, name, type, pt, dt);

      if (! cur) {
	    cur = new PWire(name, type, pt, dt);
	    FILE_NAME(cur, li.text, li.first_line);
      }

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
	    cur->set_range_scalar(SR_NET);
	    cur->set_signed(true);
	    break;
	  default:
	    break;
      }

      if (attr) {
	    for (list<named_pexpr_t>::iterator attr_cur = attr->begin()
		       ; attr_cur != attr->end() ;  ++attr_cur) {
		  cur->attributes[attr_cur->name] = attr_cur->parm;
	    }
      }
}

/*
 * This form takes a list of names and some type information, and
 * generates a bunch of variables/nets. We use the basic
 * pform_makewire above.
 */
void pform_makewire(const vlltype&li,
		    list<index_component_t>*range,
		    bool signed_flag,
		    list<perm_string>*names,
		    NetNet::Type type,
		    NetNet::PortType pt,
		    ivl_variable_type_t dt,
		    list<named_pexpr_t>*attr,
		    PWSRType rt)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_makewire(li, txt, type, pt, dt, attr);
	    /* This has already been done for real variables. */
	    if (dt != IVL_VT_REAL) {
		  pform_set_net_range(txt, range, signed_flag, dt, rt);
	    }
      }

      delete names;
      delete range;
}

/*
 * This form makes nets with delays and continuous assignments.
 */
void pform_makewire(const vlltype&li,
		    list<index_component_t>*range,
		    bool signed_flag,
		    list<PExpr*>*delay,
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
		                      SR_NET);
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
svector<PWire*>*pform_make_task_ports(const struct vlltype&loc,
				      NetNet::PortType pt,
				      ivl_variable_type_t vtype,
				      bool signed_flag,
				      list<index_component_t>*range,
				      list<perm_string>*names,
				      bool isint)
{
      assert(pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);
      assert(names);
      svector<PWire*>*res = new svector<PWire*>(0);
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {

	    perm_string name = *cur;

	      /* Look for a preexisting wire. If it exists, set the
		 port direction. If not, create it. */
	    PWire*curw = pform_get_wire_in_scope(name);
	    if (curw) {
		  curw->set_port_type(pt);
	    } else {
		  curw = new PWire(name, NetNet::IMPLICIT_REG, pt, vtype);
		  FILE_NAME(curw, loc);
		  pform_put_wire_in_scope(name, curw);
	    }

	    curw->set_signed(signed_flag);
	    if (isint) assert(curw->set_wire_type(NetNet::INTEGER));

	      /* If there is a range involved, it needs to be set. */
	    if (range) {
		  list<PWire::range_t> rlist;
		  ranges_from_list(rlist, range);
		  curw->set_range(rlist, SR_PORT);
	    }

	    svector<PWire*>*tmp = new svector<PWire*>(*res, curw);

	    delete res;
	    res = tmp;
      }

      delete range;
      delete names;
      return res;
}

svector<PWire*>*pform_make_task_ports(const struct vlltype&loc,
				      NetNet::PortType pt,
				      data_type_t*vtype,
				      list<perm_string>*names)
{
      if (atom2_type_t*atype = dynamic_cast<atom2_type_t*> (vtype)) {
	    list<index_component_t>*range_tmp = make_range_from_width(atype->type_code);
	    return pform_make_task_ports(loc, pt, IVL_VT_BOOL,
					 atype->signed_flag,
					 range_tmp, names);
      }

      if (vector_type_t*vec_type = dynamic_cast<vector_type_t*> (vtype)) {
	    return pform_make_task_ports(loc, pt, vec_type->base_type,
					 vec_type->signed_flag,
					 copy_range(vec_type->pdims.get()),
					 names);
      }

      if (/*real_type_t*real_type = */ dynamic_cast<real_type_t*> (vtype)) {
	    return pform_make_task_ports(loc, pt, IVL_VT_REAL,
					 true, 0, names);
      }

      VLerror(loc, "sorry: Given type not supported here.");
      return 0;
}

/*
 * The parser calls this in the rule that matches increment/decrement
 * statements. The rule that does the matching creates a PEUnary with
 * all the information we need, but here we convert that expression to
 * a compressed assignment statement.
 */
PAssign* pform_compressed_assign_from_inc_dec(const struct vlltype&loc, PExpr*exp)
{
      PEUnary*expu = dynamic_cast<PEUnary*> (exp);
      ivl_assert(*exp, expu != 0);

      char use_op = 0;
      switch (expu->get_op()) {
	  case 'i':
	  case 'I':
	    use_op = '+';
	    break;
	  case 'd':
	  case 'D':
	    use_op = '-';
	    break;
	  default:
	    ivl_assert(*exp, 0);
	    break;
      }

      PExpr*lval = expu->get_expr();
      PExpr*rval = new PENumber(new verinum((uint64_t)1, 1));
      FILE_NAME(rval, loc);

      PAssign*tmp = new PAssign(lval, use_op, rval);
      FILE_NAME(tmp, loc);

      delete exp;
      return tmp;
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
      PWire*cur = lexical_scope->wires_find(name);
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
			 bool signed_flag, list<index_component_t>*range, PExpr*expr,
			 LexicalScope::range_t*value_range)
{
      LexicalScope*scope = lexical_scope;
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
	    assert(range->size() == 1);
	    index_component_t index = range->front();
	    assert(index.msb);
	    assert(index.lsb);
	    parm.msb = index.msb;
	    parm.lsb = index.lsb;
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
			  bool signed_flag, list<index_component_t>*range, PExpr*expr)
{
      LexicalScope*scope = lexical_scope;

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
      Module::param_expr_t&parm = scope->localparams[name];
      FILE_NAME(&parm, loc);

      parm.expr = expr;

      parm.type = type;
      if (range) {
	    assert(range->size() == 1);
	    index_component_t index = range->front();
	    assert(index.msb);
	    assert(index.lsb);
	    parm.msb = index.msb;
	    parm.lsb = index.lsb;
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
      pform_cur_module->defparms.push_back(make_pair(name,expr));
}

/*
 * Specify paths.
 */
extern PSpecPath* pform_make_specify_path(const struct vlltype&li,
					  list<perm_string>*src, char pol,
					  bool full_flag, list<perm_string>*dst)
{
      PSpecPath*path = new PSpecPath(src->size(), dst->size(), pol, full_flag);
      FILE_NAME(path, li.text, li.first_line);

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

extern PSpecPath* pform_assign_path_delay(PSpecPath*path, list<PExpr*>*del)
{
      if (path == 0)
	    return 0;

      assert(path->delays.empty());

      path->delays.resize(del->size());
      for (unsigned idx = 0 ;  idx < path->delays.size() ;  idx += 1) {
	    path->delays[idx] = del->front();
	    del->pop_front();
      }

      delete del;

      return path;
}


extern void pform_module_specify_path(PSpecPath*obj)
{
      if (obj == 0)
	    return;
      pform_cur_module->specify_paths.push_back(obj);
}


static void pform_set_port_type(perm_string name, NetNet::PortType pt,
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

void pform_set_port_type(const struct vlltype&li,
			 list<perm_string>*names,
			 list<index_component_t>*range,
			 bool signed_flag,
			 NetNet::PortType pt)
{
      assert(pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);

      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_port_type(txt, pt, li.text, li.first_line);
	    pform_set_net_range(txt, range, signed_flag, IVL_VT_NO_TYPE,
	                        SR_PORT);
      }

      delete names;
      delete range;
}

static void pform_set_reg_integer(perm_string name)
{
      PWire*cur = pform_get_make_wire_in_scope(name, NetNet::INTEGER, NetNet::NOT_A_PORT, IVL_VT_LOGIC);
      assert(cur);

      PWire::range_t rng;
      rng.msb = new PENumber(new verinum(integer_width-1, integer_width));
      rng.lsb = new PENumber(new verinum((uint64_t)0, integer_width));
      list<PWire::range_t>rlist;
      rlist.push_back(rng);
      cur->set_range(rlist, SR_NET);
      cur->set_signed(true);
}

void pform_set_reg_integer(list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_reg_integer(txt);
      }
      delete names;
}

static void pform_set_reg_time(perm_string name)
{
      PWire*cur = pform_get_make_wire_in_scope(name, NetNet::REG, NetNet::NOT_A_PORT, IVL_VT_LOGIC);
      assert(cur);

      PWire::range_t rng;
      rng.msb = new PENumber(new verinum(TIME_WIDTH-1, integer_width));
      rng.lsb = new PENumber(new verinum((uint64_t)0, integer_width));
      list<PWire::range_t>rlist;
      rlist.push_back(rng);
      cur->set_range(rlist, SR_NET);
}

void pform_set_reg_time(list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_reg_time(txt);
      }
      delete names;
}

static void pform_set_integer_2atom(uint64_t width, bool signed_flag, perm_string name)
{
      PWire*cur = pform_get_make_wire_in_scope(name, NetNet::REG, NetNet::NOT_A_PORT, IVL_VT_BOOL);
      assert(cur);

      cur->set_signed(signed_flag);

      PWire::range_t rng;
      rng.msb = new PENumber(new verinum(width-1, integer_width));
      rng.lsb = new PENumber(new verinum((uint64_t)0, integer_width));
      list<PWire::range_t>rlist;
      rlist.push_back(rng);
      cur->set_range(rlist, SR_NET);
}

void pform_set_integer_2atom(uint64_t width, bool signed_flag, list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_integer_2atom(width, signed_flag, txt);
      }
      delete names;
}

/*
 * This function detects the derived class for the given type and
 * dispatches the type to the proper subtype function.
 */
void pform_set_data_type(const struct vlltype&li, data_type_t*data_type, list<perm_string>*names)
{
      if (atom2_type_t*atom2_type = dynamic_cast<atom2_type_t*> (data_type)) {
	    pform_set_integer_2atom(atom2_type->type_code, atom2_type->signed_flag, names);
	    return;
      }

      if (struct_type_t*struct_type = dynamic_cast<struct_type_t*> (data_type)) {
	    pform_set_struct_type(struct_type, names);
	    return;
      }

      if (enum_type_t*enum_type = dynamic_cast<enum_type_t*> (data_type)) {
	    pform_set_enum(li, enum_type, names);
	    return;
      }

      if (vector_type_t*vec_type = dynamic_cast<vector_type_t*> (data_type)) {
	    pform_set_net_range(names, vec_type->pdims.get(),
				vec_type->signed_flag,
				vec_type->base_type);
	    return;
      }

      if (/*real_type_t*real_type =*/ dynamic_cast<real_type_t*> (data_type)) {
	    pform_set_net_range(names, 0, true, IVL_VT_REAL);
	    return;
      }

      if (/*class_type_t*class_type =*/ dynamic_cast<class_type_t*> (data_type)) {
	    VLerror(li, "sorry: Class types not supported.");
	    return;
      }

      assert(0);
}

static void pform_set_enum(const struct vlltype&li, enum_type_t*enum_type,
			   perm_string name)
{
      (void) li; // The line information is not currently needed.
      PWire*cur = pform_get_make_wire_in_scope(name, NetNet::REG, NetNet::NOT_A_PORT, enum_type->base_type);
      assert(cur);

      cur->set_signed(enum_type->signed_flag);

      assert(enum_type->range.get() != 0);
      assert(enum_type->range->size() == 1);
      list<PWire::range_t>rlist;
      ranges_from_list(rlist, enum_type->range.get());
      cur->set_range(rlist, SR_NET);
      cur->set_enumeration(enum_type);
}

void pform_set_enum(const struct vlltype&li, enum_type_t*enum_type, list<perm_string>*names)
{
	// By definition, the base type can only be IVL_VT_LOGIC or
	// IVL_VT_BOOL.
      assert(enum_type->base_type==IVL_VT_LOGIC || enum_type->base_type==IVL_VT_BOOL);

      assert(enum_type->range.get() != 0);
      assert(enum_type->range->size() == 1);

	// Add the file and line information to the enumeration type.
      FILE_NAME(&(enum_type->li), li);

	// Attach the enumeration to the current scope.
      pform_put_enum_type_in_scope(enum_type);

	// Now apply the checked enumeration type to the variables
	// that are being declared with this type.
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur) {
	    perm_string txt = *cur;
	    pform_set_enum(li, enum_type, txt);
      }

      delete names;
}

svector<PWire*>* pform_make_udp_input_ports(list<perm_string>*names)
{
      svector<PWire*>*out = new svector<PWire*>(names->size());

      unsigned idx = 0;
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
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
			      list<named_pexpr_t>*attr)
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
