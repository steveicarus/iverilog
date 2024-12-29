/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <cstdarg>
# include  "compiler.h"
# include  "pform.h"
# include  "parse_misc.h"
# include  "parse_api.h"
# include  "PClass.h"
# include  "PEvent.h"
# include  "PPackage.h"
# include  "PUdp.h"
# include  "PGenerate.h"
# include  "PModport.h"
# include  "PSpec.h"
# include  "PTimingCheck.h"
# include  "discipline.h"
# include  <list>
# include  <map>
# include  <cassert>
# include  <stack>
# include  <typeinfo>
# include  <sstream>
# include  <cstring>
# include  <cstdlib>
# include  <cctype>

# include  "ivl_assert.h"
# include  "ivl_alloc.h"

using namespace std;

/*
 * The "// synthesis translate_on/off" meta-comments cause this flag
 * to be turned off or on. The pform_make_behavior and similar
 * functions look at this flag and may choose to add implicit ivl
 * synthesis flags.
 */
static bool pform_mc_translate_flag = true;
void pform_mc_translate_on(bool flag) { pform_mc_translate_flag = flag; }

/*
 * The pform_modules is a map of the modules that have been defined in
 * the top level. This should not contain nested modules/programs.
 * pform_primitives is similar, but for UDP primitives.
 */
map<perm_string,Module*> pform_modules;
map<perm_string,PUdp*> pform_primitives;

/*
 * The pform_units is a list of the SystemVerilog compilation unit scopes.
 * The current compilation unit is the last element in the list. All items
 * declared or defined at the top level (outside any design element) are
 * added to the current compilation unit scope.
 */
vector<PPackage*> pform_units;

static bool is_compilation_unit(LexicalScope*scope)
{
	// A compilation unit is the only scope that doesn't have a parent.
      assert(scope);
      return scope->parent_scope() == 0;
}

std::string vlltype::get_fileline() const
{
      ostringstream buf;
      buf << (text? text : "") << ":" << first_line;
      string res = buf.str();
      return res;

}

static bool is_hex_digit_str(const char *str)
{
      while (*str) {
	    if (!isxdigit(*str)) return false;
	    str++;
      }
      return true;
}

static bool is_dec_digit_str(const char *str)
{
      while (*str) {
	    if (!isdigit(*str)) return false;
	    str++;
      }
      return true;
}

static bool is_oct_digit_str(const char *str)
{
      while (*str) {
	    if (*str < '0' || *str > '7') return false;
	    str++;
      }
      return true;
}

static bool is_bin_digit_str(const char *str)
{
      while (*str) {
	    if (*str != '0' && *str != '1') return false;
	    str++;
      }
      return true;
}

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
    free(key);

    // Resolve value to PExpr class. Should support all kind of constant
    // format including based number, dec number, real number and string.

    // Is it a string?
    if (*value == '"') {
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
	if (*buf_ptr == '\0')	// String end without '"'
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
	free(value);
	return;
    }

    // Is it a based number?
    char *num = strchr(value, '\'');
    if (num != 0) {
	verinum *val;
	const char *base = num + 1;
	if (*base == 's' || *base == 'S')
	    base++;
	switch (*base) {
	  case 'h':
	  case 'H':
	    if (is_hex_digit_str(base+1)) {
		val = make_unsized_hex(num);
	    } else {
		cerr << "<command line>: error: invalid digit in hex value specified for defparam: " << name << endl;
		free(value);
		return;
	    }
	    break;
	  case 'd':
	  case 'D':
	    if (is_dec_digit_str(base+1)) {
		val = make_unsized_dec(num);
	    } else {
		cerr << "<command line>: error: invalid digit in decimal value specified for defparam: " << name << endl;
		free(value);
		return;
	    }
	    break;
	  case 'o':
	  case 'O':
	    if (is_oct_digit_str(base+1)) {
		val = make_unsized_octal(num);
	    } else {
		cerr << "<command line>: error: invalid digit in octal value specified for defparam: " << name << endl;
		free(value);
		return;
	    }
	    break;
	  case 'b':
	  case 'B':
	    if (is_bin_digit_str(base+1)) {
		val = make_unsized_binary(num);
	    } else {
		cerr << "<command line>: error: invalid digit in binary value specified for defparam: " << name << endl;
		free(value);
		return;
	    }
	    break;
	  default:
	    cerr << "<command line>: error: invalid numeric base specified for defparam: " << name << endl;
	    free(value);
	    return;
	}
	if (num != value) {  // based number with size
	    *num = 0;
	    if (is_dec_digit_str(value)) {
		verinum *siz = make_unsized_dec(value);
		val = pform_verinum_with_size(siz, val, "<command line>", 0);
	    } else {
		cerr << "<command line>: error: invalid size for value specified for defparam: " << name << endl;
		free(value);
		return;
	    }
	}
	PExpr* ndec = new PENumber(val);
	Module::user_defparms.push_back( make_pair(name, ndec) );
	free(value);
	return;
    }

    // Is it a decimal number?
    num = (value[0] == '-') ? value + 1 : value;
    if (num[0] != '\0' && is_dec_digit_str(num)) {
	verinum *val = make_unsized_dec(num);
	if (value[0] == '-') *val = -(*val);
	PExpr* ndec = new PENumber(val);
	Module::user_defparms.push_back( make_pair(name, ndec) );
	free(value);
	return;
    }

    // Is it a real number?
    char *end = 0;
    double rval = strtod(value, &end);
    if (end != value && *end == 0) {
	verireal *val = new verireal(rval);
	PExpr* nreal = new PEFNumber(val);
	Module::user_defparms.push_back( make_pair(name, nreal) );
	free(value);
	return;
    }

    // None of the above.
    cerr << "<command line>: error: invalid value specified for defparam: " << name << endl;
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
static list<Module*>pform_cur_module;

bool pform_library_flag = false;

/*
 * Give each unnamed block that has a variable declaration a unique name.
 */
static unsigned scope_unnamed_block_with_decl = 1;

  /* This tracks the current generate scheme being processed. This is
     always within a module. */
static PGenerate*pform_cur_generate = 0;

  /* This indicates whether a new generate construct can be directly
     nested in the current generate construct. */
bool pform_generate_single_item = false;

  /* Blocks within the same conditional generate construct may have
     the same name. Here we collect the set of names used in each
     construct, so they can be added to the local scope without
     conflicting with each other. Generate constructs may nest, so
     we need a stack. */
static list<set<perm_string> > conditional_block_names;

  /* This tracks the current modport list being processed. This is
     always within an interface. */
static PModport*pform_cur_modport = 0;

static NetNet::Type pform_default_nettype = NetNet::WIRE;

/*
 * These variables track the time scale set by the most recent `timescale
 * directive. Time scales set by SystemVerilog timeunit and timeprecision
 * declarations are stored directly in the current lexical scope.
 */
static int pform_time_unit;
static int pform_time_prec;

/*
 * These variables track where the most recent `timescale directive
 * occurred. This allows us to warn about time scales that are inherited
 * from another file.
 */
static char*pform_timescale_file = 0;
static unsigned pform_timescale_line;

/*
 * These variables track whether we can accept new timeunits declarations.
 */
bool allow_timeunit_decl = true;
bool allow_timeprec_decl = true;

// Track whether the current parameter declaration is in a parameter port list
static bool pform_in_parameter_port_list = false;

/*
 * The lexical_scope keeps track of the current lexical scope that is
 * being parsed. The lexical scope may stack, so the current scope may
 * have a parent, that is restored when the current scope ends.
 *
 * Items that have scoped names are put in the lexical_scope object.
 */
static LexicalScope* lexical_scope = 0;

LexicalScope* pform_peek_scope(void)
{
      assert(lexical_scope);
      return lexical_scope;
}

static void pform_check_possible_imports(LexicalScope *scope)
{
      map<perm_string,PPackage*>::const_iterator cur;
      for (cur = scope->possible_imports.begin(); cur != scope->possible_imports.end(); ++cur) {
            if (scope->local_symbols.find(cur->first) == scope->local_symbols.end())
                  scope->explicit_imports[cur->first] = cur->second;
      }
      scope->possible_imports.clear();
}

void pform_pop_scope()
{
      LexicalScope*scope = lexical_scope;
      assert(scope);

      pform_check_possible_imports(scope);

      lexical_scope = scope->parent_scope();
      assert(lexical_scope);
}

static LexicalScope::lifetime_t find_lifetime(LexicalScope::lifetime_t lifetime)
{
      if (lifetime != LexicalScope::INHERITED)
	    return lifetime;

      return lexical_scope->default_lifetime;
}

static PScopeExtra* find_nearest_scopex(LexicalScope*scope)
{
      PScopeExtra*scopex = dynamic_cast<PScopeExtra*> (scope);
      while (scope && !scopex) {
	    scope = scope->parent_scope();
	    scopex = dynamic_cast<PScopeExtra*> (scope);
      }
      return scopex;
}

static void add_local_symbol(LexicalScope*scope, perm_string name, PNamedItem*item)
{
      assert(scope);

	// Check for conflict with another local symbol.
      map<perm_string,PNamedItem*>::const_iterator cur_sym
	    = scope->local_symbols.find(name);
      if (cur_sym != scope->local_symbols.end()) {
	    cerr << item->get_fileline() << ": error: "
		    "'" << name << "' has already been declared "
		    "in this scope." << endl;
	    cerr << cur_sym->second->get_fileline() << ":      : "
		    "It was declared here as "
		 << cur_sym->second->symbol_type() << "." << endl;
	    error_count += 1;
	    return;
      }

	// Check for conflict with an explicit import.
      map<perm_string,PPackage*>::const_iterator cur_pkg
	    = scope->explicit_imports.find(name);
      if (cur_pkg != scope->explicit_imports.end()) {
	    cerr << item->get_fileline() << ": error: "
		    "'" << name << "' has already been "
		    "imported into this scope from package '"
		 << cur_pkg->second->pscope_name() << "'." << endl;
	    error_count += 1;
	    return;
      }

      scope->local_symbols[name] = item;
}

static void check_potential_imports(const struct vlltype&loc, perm_string name, bool tf_call)
{
      LexicalScope*scope = lexical_scope;
      while (scope) {
	    if (scope->local_symbols.find(name) != scope->local_symbols.end())
		  return;
	    if (scope->explicit_imports.find(name) != scope->explicit_imports.end())
		  return;
	    if (pform_find_potential_import(loc, scope, name, tf_call, true))
		  return;

	    scope = scope->parent_scope();
      }
}

/*
 * Set the local time unit/precision. This version is used for setting
 * the time scale for design elements (modules, packages, etc.) and is
 * called after any initial timeunit and timeprecision declarations
 * have been parsed.
 */
void pform_set_scope_timescale(const struct vlltype&loc)
{
      PScopeExtra*scope = dynamic_cast<PScopeExtra*>(lexical_scope);
      ivl_assert(loc, scope);

      PScopeExtra*parent = find_nearest_scopex(scope->parent_scope());

      bool used_global_timescale = false;
      if (scope->time_unit_is_default) {
            if (is_compilation_unit(scope)) {
                  scope->time_unit = def_ts_units;
            } else if (!is_compilation_unit(parent)) {
                  scope->time_unit = parent->time_unit;
                  scope->time_unit_is_default = parent->time_unit_is_default;
            } else if (pform_timescale_file != 0) {
                  scope->time_unit = pform_time_unit;
                  scope->time_unit_is_default = false;
                  used_global_timescale = true;
            } else /* parent is compilation unit */ {
                  scope->time_unit = parent->time_unit;
                  scope->time_unit_is_default = parent->time_unit_is_default;
            }
      }
      if (scope->time_prec_is_default) {
            if (is_compilation_unit(scope)) {
                  scope->time_precision = def_ts_prec;
            } else if (!is_compilation_unit(parent)) {
                  scope->time_precision = parent->time_precision;
                  scope->time_prec_is_default = parent->time_prec_is_default;
            } else if (pform_timescale_file != 0) {
                  scope->time_precision = pform_time_prec;
                  scope->time_prec_is_default = false;
                  used_global_timescale = true;
            } else {
                  scope->time_precision = parent->time_precision;
                  scope->time_prec_is_default = parent->time_prec_is_default;
            }
      }

      if (gn_system_verilog() && (scope->time_unit < scope->time_precision)) {
	    if (scope->time_unit_is_local || scope->time_prec_is_local) {
		  VLerror("error: A timeprecision is missing or is too large!");
	    }
      } else {
            ivl_assert(loc, scope->time_unit >= scope->time_precision);
      }

      if (warn_timescale && used_global_timescale
	  && (strcmp(pform_timescale_file, loc.text) != 0)) {

	    cerr << loc.get_fileline() << ": warning: "
		 << "timescale for " << scope->pscope_name()
		 << " inherited from another file." << endl;
	    cerr << pform_timescale_file << ":" << pform_timescale_line
		 << ": ...: The inherited timescale is here." << endl;
      }

      allow_timeunit_decl = false;
      allow_timeprec_decl = false;
}

/*
 * Set the local time unit/precision. This version is used for setting
 * the time scale for subsidiary items (classes, subroutines, etc.),
 * which simply inherit their time scale from their parent scope.
 */
static void pform_set_scope_timescale(PScope*scope, const PScope*parent)
{
      scope->time_unit            = parent->time_unit;
      scope->time_precision       = parent->time_precision;
      scope->time_unit_is_default = parent->time_unit_is_default;
      scope->time_prec_is_default = parent->time_prec_is_default;
}

PClass* pform_push_class_scope(const struct vlltype&loc, perm_string name)
{
      PClass*class_scope = new PClass(name, lexical_scope);
      class_scope->default_lifetime = LexicalScope::AUTOMATIC;
      FILE_NAME(class_scope, loc);

      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      ivl_assert(loc, scopex);
      ivl_assert(loc, !pform_cur_generate);

      pform_set_scope_timescale(class_scope, scopex);

      scopex->classes[name] = class_scope;
      scopex->classes_lexical .push_back(class_scope);

      lexical_scope = class_scope;
      return class_scope;
}

PPackage* pform_push_package_scope(const struct vlltype&loc, perm_string name,
				   LexicalScope::lifetime_t lifetime)
{
      PPackage*pkg_scope = new PPackage(name, lexical_scope);
      pkg_scope->default_lifetime = find_lifetime(lifetime);
      FILE_NAME(pkg_scope, loc);

      allow_timeunit_decl = true;
      allow_timeprec_decl = true;

      lexical_scope = pkg_scope;
      return pkg_scope;
}

PTask* pform_push_task_scope(const struct vlltype&loc, char*name,
			     LexicalScope::lifetime_t lifetime)
{
      perm_string task_name = lex_strings.make(name);

      LexicalScope::lifetime_t default_lifetime = find_lifetime(lifetime);
      bool is_auto = default_lifetime == LexicalScope::AUTOMATIC;

      PTask*task = new PTask(task_name, lexical_scope, is_auto);
      task->default_lifetime = default_lifetime;
      FILE_NAME(task, loc);

      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      ivl_assert(loc, scopex);
      if (is_compilation_unit(scopex) && !gn_system_verilog()) {
	    cerr << task->get_fileline() << ": error: task declarations "
		  "must be contained within a module." << endl;
	    error_count += 1;
      }

      pform_set_scope_timescale(task, scopex);

      if (pform_cur_generate) {
	    add_local_symbol(pform_cur_generate, task_name, task);
	    pform_cur_generate->tasks[task_name] = task;
      } else {
	    add_local_symbol(scopex, task_name, task);
	    scopex->tasks[task_name] = task;
      }

      lexical_scope = task;

      return task;
}

PFunction* pform_push_function_scope(const struct vlltype&loc, const char*name,
                                     LexicalScope::lifetime_t lifetime)
{
      perm_string func_name = lex_strings.make(name);

      LexicalScope::lifetime_t default_lifetime = find_lifetime(lifetime);
      bool is_auto = default_lifetime == LexicalScope::AUTOMATIC;

      PFunction*func = new PFunction(func_name, lexical_scope, is_auto);
      func->default_lifetime = default_lifetime;
      FILE_NAME(func, loc);

      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      ivl_assert(loc, scopex);
      if (is_compilation_unit(scopex) && !gn_system_verilog()) {
	    cerr << func->get_fileline() << ": error: function declarations "
		  "must be contained within a module." << endl;
	    error_count += 1;
      }

      pform_set_scope_timescale(func, scopex);

      if (pform_cur_generate) {
	    add_local_symbol(pform_cur_generate, func_name, func);
	    pform_cur_generate->funcs[func_name] = func;

      } else {
	    add_local_symbol(scopex, func_name, func);
	    scopex->funcs[func_name] = func;
      }

      lexical_scope = func;

      return func;
}

PBlock* pform_push_block_scope(const struct vlltype&loc, char*name,
			       PBlock::BL_TYPE bt)
{
      perm_string block_name;
      if (name) block_name = lex_strings.make(name);
      else {
	      // Create a unique name for this unnamed block.
	    char tmp[32];
	    snprintf(tmp, sizeof tmp, "$unm_blk_%u",
	             scope_unnamed_block_with_decl);
	    block_name = lex_strings.make(tmp);
	    scope_unnamed_block_with_decl += 1;
      }

      PBlock*block = new PBlock(block_name, lexical_scope, bt);
      FILE_NAME(block, loc);
      block->default_lifetime = find_lifetime(LexicalScope::INHERITED);
      if (name) add_local_symbol(lexical_scope, block_name, block);
      lexical_scope = block;

      return block;
}

/*
 * Create a new identifier.
 */
PEIdent* pform_new_ident(const struct vlltype&loc, const pform_name_t&name)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, false);

      return new PEIdent(name, loc.lexical_pos);
}

PTrigger* pform_new_trigger(const struct vlltype&loc, PPackage*pkg,
			    const pform_name_t&name, unsigned lexical_pos)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, false);

      PTrigger*tmp = new PTrigger(pkg, name, lexical_pos);
      FILE_NAME(tmp, loc);
      return tmp;
}

PNBTrigger* pform_new_nb_trigger(const struct vlltype&loc,
			         const list<PExpr*>*dly,
			         const pform_name_t&name,
			         unsigned lexical_pos)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, false);

      PExpr*tmp_dly = 0;
      if (dly) {
	    ivl_assert(loc, dly->size() == 1);
	    tmp_dly = dly->front();
      }

      PNBTrigger*tmp = new PNBTrigger(name, lexical_pos, tmp_dly);
      FILE_NAME(tmp, loc);
      return tmp;
}

PGenerate* pform_parent_generate(void)
{
      return pform_cur_generate;
}

bool pform_error_in_generate(const vlltype&loc, const char *type)
{
	if (!pform_parent_generate())
		return false;

	VLerror(loc, "error: %s is not allowed in generate block.", type);
	return true;
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

bool pform_in_program_block()
{
      if (pform_cur_module.empty())
	    return false;
      if (pform_cur_module.front()->program_block)
	    return true;
      return false;
}

bool pform_in_interface()
{
      if (pform_cur_module.empty())
	    return false;
      if (pform_cur_module.front()->is_interface)
	    return true;
      return false;
}

static bool pform_at_module_level()
{
      return (lexical_scope == pform_cur_module.front())
          || (lexical_scope == pform_cur_generate);
}

PWire*pform_get_wire_in_scope(perm_string name)
{
      return lexical_scope->wires_find(name);
}

static void pform_put_wire_in_scope(perm_string name, PWire*net)
{
      add_local_symbol(lexical_scope, name, net);
      lexical_scope->wires[name] = net;
}

void pform_put_enum_type_in_scope(enum_type_t*enum_set)
{
      if (std::find(lexical_scope->enum_sets.begin(),
		    lexical_scope->enum_sets.end(), enum_set) !=
          lexical_scope->enum_sets.end())
	    return;

      set<perm_string> enum_names;
      list<named_pexpr_t>::const_iterator cur;
      for (cur = enum_set->names->begin(); cur != enum_set->names->end(); ++cur) {
	    if (enum_names.count(cur->name)) {
		  cerr << enum_set->get_fileline() << ": error: "
			  "Duplicate enumeration name '"
		       << cur->name << "'." << endl;
		  error_count += 1;
	    } else {
		  add_local_symbol(lexical_scope, cur->name, enum_set);
		  enum_names.insert(cur->name);
	    }
      }

      lexical_scope->enum_sets.push_back(enum_set);
}

static typedef_t *pform_get_typedef(const struct vlltype&loc, perm_string name)
{
      typedef_t *&td = lexical_scope->typedefs[name];
      if (!td) {
	    td = new typedef_t(name);
	    FILE_NAME(td, loc);
	    add_local_symbol(lexical_scope, name, td);
      }
      return td;
}

void pform_forward_typedef(const struct vlltype&loc, perm_string name,
			   enum typedef_t::basic_type basic_type)
{
      typedef_t *td = pform_get_typedef(loc, name);

      if (!td->set_basic_type(basic_type)) {
	    cout << loc << " error: Incompatible basic type `" << basic_type
	         << "` for `" << name
		 << "`. Previously declared in this scope as `"
		 << td->get_basic_type() << "` at " << td->get_fileline() << "."
	         << endl;
	    error_count++;
      }
}

void pform_set_typedef(const struct vlltype&loc, perm_string name,
		       data_type_t*data_type,
		       std::list<pform_range_t>*unp_ranges)
{
      typedef_t *td = pform_get_typedef(loc, name);

      if(unp_ranges)
	    data_type = new uarray_type_t(data_type, unp_ranges);

      if (!td->set_data_type(data_type)) {
	    cerr << loc << " error: Type identifier `" << name
		 << "` has already been declared in this scope at "
		 << td->get_data_type()->get_fileline() << "."
		 << endl;
	    error_count++;
	    delete data_type;
      }
}

void pform_set_type_referenced(const struct vlltype&loc, const char*name)
{
      perm_string lex_name = lex_strings.make(name);
      check_potential_imports(loc, lex_name, false);
}

typedef_t* pform_test_type_identifier(const struct vlltype&loc, const char*txt)
{
      perm_string name = lex_strings.make(txt);

      LexicalScope*cur_scope = lexical_scope;
      do {
	    LexicalScope::typedef_map_t::iterator cur;

	      // First look to see if this identifier is imported from
	      // a package. If it is, see if it is a type in that
	      // package. If it is, then great. If imported as
	      // something other than a type, then give up now because
	      // the name has at least shadowed any other possible
	      // meaning for this name.
	    map<perm_string,PPackage*>::iterator cur_pkg;
	    cur_pkg = cur_scope->explicit_imports.find(name);
	    if (cur_pkg != cur_scope->explicit_imports.end()) {
		  PPackage*pkg = cur_pkg->second;
		  cur = pkg->typedefs.find(name);
		  if (cur != pkg->typedefs.end())
			return cur->second;

		    // Not a type. Give up.
		  return 0;
	    }

	    cur = cur_scope->typedefs.find(name);
	    if (cur != cur_scope->typedefs.end())
		  return cur->second;

            PPackage*pkg = pform_find_potential_import(loc, cur_scope, name, false, false);
            if (pkg) {
	          cur = pkg->typedefs.find(name);
	          if (cur != pkg->typedefs.end())
		        return cur->second;

		    // Not a type. Give up.
		  return 0;
            }

	    cur_scope = cur_scope->parent_scope();
      } while (cur_scope);

      return 0;
}

PECallFunction* pform_make_call_function(const struct vlltype&loc,
					 const pform_name_t&name,
					 const list<named_pexpr_t> &parms)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, true);

      PECallFunction*tmp = new PECallFunction(name, parms);
      FILE_NAME(tmp, loc);
      return tmp;
}

PCallTask* pform_make_call_task(const struct vlltype&loc,
				const pform_name_t&name,
				const list<named_pexpr_t> &parms)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, true);

      PCallTask*tmp = new PCallTask(name, parms);
      FILE_NAME(tmp, loc);
      return tmp;
}

void pform_make_var(const struct vlltype&loc,
		    std::list<decl_assignment_t*>*assign_list,
		    data_type_t*data_type, std::list<named_pexpr_t>*attr,
		    bool is_const)
{
      static const struct str_pair_t str = { IVL_DR_STRONG, IVL_DR_STRONG };

      pform_makewire(loc, 0, str, assign_list, NetNet::REG, data_type, attr,
		     is_const);
}

void pform_make_foreach_declarations(const struct vlltype&loc,
				     std::list<perm_string>*loop_vars)
{
      list<decl_assignment_t*>assign_list;
      for (list<perm_string>::const_iterator cur = loop_vars->begin()
		 ; cur != loop_vars->end() ; ++ cur) {
	    if (cur->nil())
		  continue;
	    decl_assignment_t*tmp_assign = new decl_assignment_t;
	    tmp_assign->name = { lex_strings.make(*cur), 0 };
	    assign_list.push_back(tmp_assign);
      }

      pform_make_var(loc, &assign_list, &size_type);
}

PForeach* pform_make_foreach(const struct vlltype&loc,
			     char*name,
			     list<perm_string>*loop_vars,
			     Statement*stmt)
{
      perm_string use_name = lex_strings.make(name);
      delete[]name;

      if (loop_vars==0 || loop_vars->empty()) {
	    cerr << loc.get_fileline() << ": error: "
		 << "No loop variables at all in foreach index." << endl;
	    error_count += 1;
      }

      ivl_assert(loc, loop_vars);
      PForeach*fe = new PForeach(use_name, *loop_vars, stmt);
      FILE_NAME(fe, loc);

      delete loop_vars;

      return fe;
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

      if (! pform_cur_module.empty()) {
	    cerr << file<<":"<<lineno << ": error: "
		 << "`default_nettype directives must appear" << endl;
	    cerr << file<<":"<<lineno << ":      : "
		 << "outside module definitions. The containing" << endl;
	    cerr << file<<":"<<lineno << ":      : "
		 << "module " << pform_cur_module.back()->mod_name()
		 << " starts on line "
		 << pform_cur_module.back()->get_fileline() << "." << endl;
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
 * values (for use by subsequent design elements) and if warnings are
 * enabled checks to see if some design elements have no timescale.
 */
void pform_set_timescale(int unit, int prec,
			 const char*file, unsigned lineno)
{
      assert(unit >= prec);
      pform_time_unit = unit;
      pform_time_prec = prec;

      if (pform_timescale_file) {
	    free(pform_timescale_file);
      }

      if (file) pform_timescale_file = strdup(file);
      else pform_timescale_file = 0;
      pform_timescale_line = lineno;
}

bool get_time_unit(const char*cp, int &unit)
{
	const char *c;
	bool        rc = true;

	if (strchr(cp, '_')) {
		VLerror(yylloc, "error: Invalid timeunit constant ('_' is not "
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
		msg << "error: Invalid timeunit scale '" << cp << "'.";
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
		  VLerror(yylloc, "error: Invalid timeunit constant ('_' "
		                  "is not supported).");
	    } else {
		  VLerror(yylloc, "error: Invalid timeprecision constant ('_' "
		                  "is not supported).");
	    }
	    return true;
      }

	/* Check for the 1 digit. */
      if (*cp != '1') {
	    if (is_unit) {
		  VLerror(yylloc, "error: Invalid timeunit constant "
                                  "(1st digit).");
	    } else {
		  VLerror(yylloc, "error: Invalid timeprecision constant "
                                  "(1st digit).");
	    }
	    return true;
      }
      cp += 1;

	/* Check the number of zeros after the 1. */
      res = strspn(cp, "0");
      if (res > 2) {
	    if (is_unit) {
		  VLerror(yylloc, "error: Invalid timeunit constant "
		                  "(number of zeros).");
	    } else {
		  VLerror(yylloc, "error: Invalid timeprecision constant "
		                  "(number of zeros).");
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
      msg << "error: Invalid ";
      if (is_unit) msg << "timeunit";
      else msg << "timeprecision";
      msg << " scale '" << cp << "'.";
      VLerror(msg.str().c_str());
      return true;
}

void pform_set_timeunit(const char*txt, bool initial_decl)
{
      int val;

      if (get_time_unit_prec(txt, val, true)) return;

      PScopeExtra*scope = dynamic_cast<PScopeExtra*>(lexical_scope);
      if (!scope)
	    return;

      if (initial_decl) {
            scope->time_unit = val;
            scope->time_unit_is_local = true;
            scope->time_unit_is_default = false;
            allow_timeunit_decl = false;
      } else if (!scope->time_unit_is_local) {
            VLerror(yylloc, "error: Repeat timeunit found and the initial "
                            "timeunit for this scope is missing.");
      } else if (scope->time_unit != val) {
            VLerror(yylloc, "error: Repeat timeunit does not match the "
                            "initial timeunit for this scope.");
      }
}

int pform_get_timeunit()
{
      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      assert(scopex);
      return scopex->time_unit;
}

int pform_get_timeprec()
{
      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      assert(scopex);
      return scopex->time_precision;
}

void pform_set_timeprec(const char*txt, bool initial_decl)
{
      int val;

      if (get_time_unit_prec(txt, val, false)) return;

      PScopeExtra*scope = dynamic_cast<PScopeExtra*>(lexical_scope);
      if (!scope)
	    return;

      if (initial_decl) {
            scope->time_precision = val;
            scope->time_prec_is_local = true;
            scope->time_prec_is_default = false;
            allow_timeprec_decl = false;
      } else if (!scope->time_prec_is_local) {
            VLerror(yylloc, "error: Repeat timeprecision found and the initial "
                            "timeprecision for this scope is missing.");
      } else if (scope->time_precision != val) {
            VLerror(yylloc, "error: Repeat timeprecision does not match the "
                            "initial timeprecision for this scope.");
      }
}

verinum* pform_verinum_with_size(verinum*siz, verinum*val,
				 const char*file, unsigned lineno)
{
      assert(siz->is_defined());
      unsigned long size = siz->as_ulong();

      if (size == 0) {
	    cerr << file << ":" << lineno << ": error: Sized numeric constant "
		    "must have a size greater than zero." << endl;
	    error_count += 1;
      }

      verinum::V pad;

      if (val->len() == 0) {
	    pad = verinum::Vx;
      } else {

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

void pform_startmodule(const struct vlltype&loc, const char*name,
		       bool program_block, bool is_interface,
		       LexicalScope::lifetime_t lifetime,
		       list<named_pexpr_t>*attr)
{
      if (! pform_cur_module.empty() && !gn_system_verilog()) {
	    cerr << loc << ": error: Module definition " << name
		 << " cannot nest into module " << pform_cur_module.front()->mod_name() << "." << endl;
	    error_count += 1;
      }


      if (lifetime != LexicalScope::INHERITED) {
	    pform_requires_sv(loc, "Default subroutine lifetime");
      }

      if (gn_system_verilog() && ! pform_cur_module.empty()) {
	    if (pform_cur_module.front()->program_block) {
		  cerr << loc << ": error: module, program, or interface "
				 "declarations are not allowed in program "
				 "blocks." << endl;
		  error_count += 1;
	    }
	    if (pform_cur_module.front()->is_interface
		&& !(program_block || is_interface)) {
		  cerr << loc << ": error: module declarations are not "
				 "allowed in interfaces." << endl;
		  error_count += 1;
	    }
      }

      perm_string lex_name = lex_strings.make(name);
      Module*cur_module = new Module(lexical_scope, lex_name);
      cur_module->program_block = program_block;
      cur_module->is_interface = is_interface;
      cur_module->default_lifetime = find_lifetime(lifetime);

      FILE_NAME(cur_module, loc);

      cur_module->library_flag = pform_library_flag;

      pform_cur_module.push_front(cur_module);

      allow_timeunit_decl = true;
      allow_timeprec_decl = true;

      pform_generate_single_item = false;

      add_local_symbol(lexical_scope, lex_name, cur_module);

      lexical_scope = cur_module;

      pform_bind_attributes(cur_module->attributes, attr);
}

void pform_start_parameter_port_list()
{
      pform_in_parameter_port_list = true;
      pform_peek_scope()->has_parameter_port_list = true;
}

void pform_end_parameter_port_list()
{
      pform_in_parameter_port_list = false;
}

/*
 * This function is called by the parser to make a simple port
 * reference. This is a name without a .X(...), so the internal name
 * should be generated to be the same as the X.
 */
Module::port_t* pform_module_port_reference(const struct vlltype&loc,
					    perm_string name)
{
      Module::port_t*ptmp = new Module::port_t;
      PEIdent*tmp = new PEIdent(name, loc.lexical_pos);
      FILE_NAME(tmp, loc);
      ptmp->name = name;
      ptmp->expr.push_back(tmp);
      ptmp->default_value = 0;

      return ptmp;
}

void pform_module_set_ports(vector<Module::port_t*>*ports)
{
      assert(! pform_cur_module.empty());

	/* The parser parses ``module foo()'' as having one
	   unconnected port, but it is really a module with no
	   ports. Fix it up here. */
      if (ports && (ports->size() == 1) && ((*ports)[0] == 0)) {
	    delete ports;
	    ports = 0;
      }

      if (ports != 0) {
	    pform_cur_module.front()->ports = *ports;
	    delete ports;
      }
}

void pform_endmodule(const char*name, bool inside_celldefine,
                     Module::UCDriveType uc_drive_def)
{
	// The parser will not call pform_endmodule() without first
	// calling pform_startmodule(). Thus, it is impossible for the
	// pform_cur_module stack to be empty at this point.
      assert(! pform_cur_module.empty());
      Module*cur_module  = pform_cur_module.front();
      pform_cur_module.pop_front();
      perm_string mod_name = cur_module->mod_name();

	// Oops, there may be some sort of nesting problem. If
	// SystemVerilog is activated, it is possible for modules to
	// be nested. But if the nested module is broken, the parser
	// will recover and treat is as an invalid module item,
	// leaving the pform_cur_module stack in an inconsistent
	// state. For example, this:
	//    module foo;
	//      module bar blah blab blah error;
	//    endmodule
	// may leave the pform_cur_module stack with the dregs of the
	// bar module. Try to find the foo module in the stack, and
	// print error messages as we go.
      if (strcmp(name, mod_name) != 0) {
	    while (!pform_cur_module.empty()) {
		  Module*tmp_module = pform_cur_module.front();
		  perm_string tmp_name = tmp_module->mod_name();
		  pform_cur_module.pop_front();
		  ostringstream msg;
		  msg << "error: Module " << mod_name
		      << " was nested within " << tmp_name
		      << " but broken.";
		  VLerror(msg.str().c_str());

		  ivl_assert(*cur_module, lexical_scope == cur_module);
		  pform_pop_scope();
		  delete cur_module;

		  cur_module = tmp_module;
		  mod_name = tmp_name;
		  if (strcmp(name, mod_name) == 0)
			break;
	    }
      }
      assert(strcmp(name, mod_name) == 0);

      cur_module->is_cell = inside_celldefine;
      cur_module->uc_drive = uc_drive_def;

	// If this is a root module, then there is no parent module
	// and we try to put this newly defined module into the global
	// root list of modules. Otherwise, this is a nested module
	// and we put it into the parent module scope to be elaborated
	// if needed.
      map<perm_string,Module*>&use_module_map = (pform_cur_module.empty())
	    ? pform_modules
	    : pform_cur_module.front()->nested_modules;

      map<perm_string,Module*>::const_iterator test =
	    use_module_map.find(mod_name);

      if (test != use_module_map.end()) {
	    ostringstream msg;
	    msg << "error: Module " << name << " was already declared here: "
		<< test->second->get_fileline() << endl;
	    VLerror(msg.str().c_str());
      } else {
	    use_module_map[mod_name] = cur_module;
      }

	// The current lexical scope should be this module by now.
      ivl_assert(*cur_module, lexical_scope == cur_module);
      pform_pop_scope();
}

void pform_genvars(const struct vlltype&li, list<pform_ident_t>*names)
{
      list<pform_ident_t>::const_iterator cur;
      for (cur = names->begin(); cur != names->end() ; *cur++) {
	    PGenvar*genvar = new PGenvar();
	    FILE_NAME(genvar, li);

	    if (pform_cur_generate) {
		  add_local_symbol(pform_cur_generate, cur->first, genvar);
		  pform_cur_generate->genvars[cur->first] = genvar;
	    } else {
		  add_local_symbol(pform_cur_module.front(), cur->first, genvar);
		  pform_cur_module.front()->genvars[cur->first] = genvar;
	    }
      }

      delete names;
}

static unsigned detect_directly_nested_generate()
{
      if (pform_cur_generate && pform_generate_single_item)
	    switch (pform_cur_generate->scheme_type) {
		case PGenerate::GS_CASE_ITEM:
		  // fallthrough
		case PGenerate::GS_CONDIT:
		  // fallthrough
		case PGenerate::GS_ELSE:
		  pform_cur_generate->directly_nested = true;
		  return pform_cur_generate->id_number;
		default:
		  break;
	    }

      return ++lexical_scope->generate_counter;
}

void pform_start_generate_for(const struct vlltype&li,
			      bool local_index,
			      char*ident1, PExpr*init,
			      PExpr*test,
			      char*ident2, PExpr*next)
{
      PGenerate*gen = new PGenerate(lexical_scope, ++lexical_scope->generate_counter);
      lexical_scope = gen;

      FILE_NAME(gen, li);

      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_LOOP;

      pform_cur_generate->local_index = local_index;
      pform_cur_generate->loop_index = lex_strings.make(ident1);
      pform_cur_generate->loop_init = init;
      pform_cur_generate->loop_test = test;
      pform_cur_generate->loop_step = next;

      delete[]ident1;
      delete[]ident2;
}

void pform_start_generate_if(const struct vlltype&li, PExpr*test)
{
      unsigned id_number = detect_directly_nested_generate();

      PGenerate*gen = new PGenerate(lexical_scope, id_number);
      lexical_scope = gen;

      FILE_NAME(gen, li);

      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_CONDIT;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = test;
      pform_cur_generate->loop_step = 0;

      conditional_block_names.push_front(set<perm_string>());
}

void pform_start_generate_else(const struct vlltype&li)
{
      ivl_assert(li, pform_cur_generate);
      ivl_assert(li, pform_cur_generate->scheme_type == PGenerate::GS_CONDIT);

      PGenerate*cur = pform_cur_generate;
      pform_endgenerate(false);

      PGenerate*gen = new PGenerate(lexical_scope, cur->id_number);
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
      unsigned id_number = detect_directly_nested_generate();

      PGenerate*gen = new PGenerate(lexical_scope, id_number);
      lexical_scope = gen;

      FILE_NAME(gen, li);

      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_CASE;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = expr;
      pform_cur_generate->loop_step = 0;

      conditional_block_names.push_front(set<perm_string>());
}

/*
 * The named block generate case.
 */
void pform_start_generate_nblock(const struct vlltype&li, char*name)
{
      PGenerate*gen = new PGenerate(lexical_scope, ++lexical_scope->generate_counter);
      lexical_scope = gen;

      FILE_NAME(gen, li);

      pform_cur_generate = gen;

      pform_cur_generate->scheme_type = PGenerate::GS_NBLOCK;

      pform_cur_generate->loop_init = 0;
      pform_cur_generate->loop_test = 0;
      pform_cur_generate->loop_step = 0;

      pform_cur_generate->scope_name = lex_strings.make(name);
      delete[]name;

      add_local_symbol(pform_cur_generate->parent_scope(),
                       pform_cur_generate->scope_name,
                       pform_cur_generate);
}

/*
 * The generate case item is a special case schema that takes its id
 * from the case schema that it is a part of. The idea is that the
 * case schema can only instantiate exactly one item, so the items
 * need not have a unique number.
 */
void pform_generate_case_item(const struct vlltype&li, list<PExpr*>*expr_list)
{
      ivl_assert(li, pform_cur_generate);
      ivl_assert(li, pform_cur_generate->scheme_type == PGenerate::GS_CASE);

      PGenerate*gen = new PGenerate(lexical_scope, pform_cur_generate->id_number);
      lexical_scope = gen;

      FILE_NAME(gen, li);

      gen->directly_nested = pform_cur_generate->directly_nested;

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
	    ivl_assert(li, expr_cur == expr_list->end());
      }
}

void pform_generate_block_name(char*name)
{
      assert(pform_cur_generate != 0);
      assert(pform_cur_generate->scope_name == 0);
      perm_string scope_name = lex_strings.make(name);
      pform_cur_generate->scope_name = scope_name;

      if (pform_cur_generate->scheme_type == PGenerate::GS_CONDIT
       || pform_cur_generate->scheme_type == PGenerate::GS_ELSE
       || pform_cur_generate->scheme_type == PGenerate::GS_CASE_ITEM) {

            if (conditional_block_names.front().count(scope_name))
                  return;

            conditional_block_names.front().insert(scope_name);
      }

      LexicalScope*parent_scope = pform_cur_generate->parent_scope();
      assert(parent_scope);
      if (pform_cur_generate->scheme_type == PGenerate::GS_CASE_ITEM)
	      // Skip over the PGenerate::GS_CASE container.
	    parent_scope = parent_scope->parent_scope();

      add_local_symbol(parent_scope, scope_name, pform_cur_generate);
}

void pform_endgenerate(bool end_conditional)
{
      assert(pform_cur_generate != 0);
      assert(! pform_cur_module.empty());

      if (end_conditional)
            conditional_block_names.pop_front();

	// If there is no explicit block name then generate a temporary
	// name. This will be replaced by the correct name later, once
	// we know all the explicit names in the surrounding scope. If
	// the naming scheme used here is changed, PGenerate::elaborate
	// must be changed to match.
      if (pform_cur_generate->scope_name == 0) {
	    char tmp[16];
	    snprintf(tmp, sizeof tmp, "$gen%u", pform_cur_generate->id_number);
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
	    pform_cur_module.front()->generate_schemes.push_back(pform_cur_generate);
      }
      pform_cur_generate = parent_generate;
}

void pform_make_elab_task(const struct vlltype&li,
                          perm_string name,
                          const list<named_pexpr_t> &params)
{
      PCallTask*elab_task = new PCallTask(name, params);
      FILE_NAME(elab_task, li);

      lexical_scope->elab_tasks.push_back(elab_task);
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
	    cerr << res->get_fileline() << ": warning: Choosing ";
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
			      const struct vlltype&loc)
{
      const bool synchronous_flag = udp->sequential;

	/* Interpret and check the table entry strings, to make sure
	   they correspond to the inputs, output and output type. Make
	   up vectors for the fully interpreted result that can be
	   placed in the PUdp object.

	   The table strings are made up by the parser to be two or
	   three substrings separated by ':', i.e.:

	   0101:1:1  (synchronous device entry)
	   0101:0    (combinational device entry)

	   The parser doesn't check that we got the right kind here,
	   so this loop must watch out. */
      std::vector<string> &input   = udp->tinput;
      std::vector<char>   &current = udp->tcurrent;
      std::vector<char>   &output  = udp->toutput;

      input.resize(table->size());
      current.resize(table->size());
      output.resize(table->size());

      { unsigned idx = 0;
        for (list<string>::iterator cur = table->begin() ;
             cur != table->end() ; ++cur , idx += 1) {
	      string tmp = *cur;

		/* Pull the input values from the string. */
	      if (tmp.find(':') != (udp->ports.size()-1)) {
		    cerr << loc << ": error: "
		         << "The UDP input port count (" << (udp->ports.size()-1)
		         << ") does not match the number of input table entries ("
		         << tmp.find(':') << ") in primitive \""
		         << udp->name_ << "\"." << endl;
		    error_count += 1;
		    break;
	      }
	      input[idx] = tmp.substr(0, udp->ports.size()-1);
	      tmp = tmp.substr(udp->ports.size()-1);


		/* If this is a synchronous device, get the current
		   output string. */
	      if (synchronous_flag) {
		    assert(tmp[0] == ':');
		    assert(tmp.size() == 4);
		    current[idx] = tmp[1];
		    tmp = tmp.substr(2);

	      }

		/* Finally, extract the desired output. */
	      assert(tmp[0] == ':');
	      assert(tmp.size() == 2);
	      output[idx] = tmp[1];
	}
      }

}

void pform_make_udp(const struct vlltype&loc, perm_string name,
		    list<pform_ident_t>*parms, vector<PWire*>*decl,
		    list<string>*table, Statement*init_expr)
{
      unsigned local_errors = 0;
      ivl_assert(loc, !parms->empty());

      ivl_assert(loc, decl);

	/* Put the declarations into a map, so that I can check them
	   off with the parameters in the list. If the port is already
	   in the map, merge the port type. I will rebuild a list
	   of parameters for the PUdp object. */
      map<perm_string,PWire*> defs;
      for (unsigned idx = 0 ;  idx < decl->size() ;  idx += 1) {

	    perm_string port_name = (*decl)[idx]->basename();

	    if (PWire*cur = defs[port_name]) {
		  ivl_assert(loc, (*decl)[idx]);
		  if ((*decl)[idx]->get_port_type() != NetNet::PIMPLICIT) {
			bool rc = cur->set_port_type((*decl)[idx]->get_port_type());
			ivl_assert(loc, rc);
		  }
		  if ((*decl)[idx]->get_wire_type() != NetNet::IMPLICIT) {
			bool rc = cur->set_wire_type((*decl)[idx]->get_wire_type());
			ivl_assert(loc, rc);
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
      std::vector<PWire*> pins(parms->size());
      std::vector<perm_string> pin_names(parms->size());
      { list<pform_ident_t>::iterator cur;
        unsigned idx;
        for (cur = parms->begin(), idx = 0
		   ; cur != parms->end()
		   ; ++ idx, ++ cur) {
	      pins[idx] = defs[cur->first];
	      pin_names[idx] = cur->first;
	}
      }

	/* Check that the output is an output and the inputs are
	   inputs. I can also make sure that only the single output is
	   declared a register, if anything. The possible errors are:

	      -- an input port (not the first) is missing an input
	         declaration.

	      -- An input port is declared output.

	*/
      ivl_assert(loc, pins.size() > 0);
      do {
	    if (pins[0] == 0) {
		  cerr << loc << ": error: "
		       << "Output port of primitive " << name
		       << " missing output declaration." << endl;
		  cerr << loc << ":      : "
		       << "Try: output " << pin_names[0] << ";"
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  break;
	    }
	    if (pins[0]->get_port_type() != NetNet::POUTPUT) {
		  cerr << loc << ": error: "
		       << "The first port of a primitive"
		       << " must be an output." << endl;
		  cerr << loc << ":      : "
		       << "Try: output " << pin_names[0] << ";"
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  break;;
	    }
      } while (0);

      for (unsigned idx = 1 ;  idx < pins.size() ;  idx += 1) {
	    if (pins[idx] == 0) {
		  cerr << loc << ": error: "
		       << "Port " << (idx+1)
		       << " of primitive " << name << " missing"
		       << " input declaration." << endl;
		  cerr << loc << ":      : "
		       << "Try: input " << pin_names[idx] << ";"
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  continue;
	    }
	    if (pins[idx]->get_port_type() != NetNet::PINPUT) {
		  cerr << loc << ": error: "
		       << "Input port " << (idx+1)
		       << " of primitive " << name
		       << " has an output (or missing) declaration." << endl;
		  cerr << loc << ":      : "
		       << "Note that only the first port can be an output."
		       << endl;
		  cerr << loc << ":      : "
		       << "Try \"input " << name << ";\""
		       << endl;
		  error_count += 1;
		  local_errors += 1;
		  continue;
	    }

	    if (pins[idx]->get_wire_type() == NetNet::REG) {
		  cerr << loc << ": error: "
		       << "Port " << (idx+1)
		       << " of primitive " << name << " is an input port"
		       << " with a reg declaration." << endl;
		  cerr << loc << ":      : "
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
	    ivl_assert(loc, pins[0]->get_wire_type() == NetNet::REG);

	    PAssign*pa = dynamic_cast<PAssign*>(init_expr);
	    ivl_assert(*init_expr, pa);

	    const PEIdent*id = dynamic_cast<const PEIdent*>(pa->lval());
	    ivl_assert(*init_expr, id);

	      // XXXX
	      //ivl_assert(*init_expr, id->name() == pins[0]->name());

	    const PENumber*np = dynamic_cast<const PENumber*>(pa->rval());
	    ivl_assert(*init_expr, np);

	    init = np->value()[0];
      }

	// Put the primitive into the primitives table
      if (pform_primitives[name]) {
	    VLwarn("warning: UDP primitive already exists.");

      } else {
	    PUdp*udp = new PUdp(name, parms->size());
	    FILE_NAME(udp, loc);

	      // Detect sequential udp.
	    if (pins[0]->get_wire_type() == NetNet::REG)
		  udp->sequential = true;

	      // Make the port list for the UDP
	    for (unsigned idx = 0 ;  idx < pins.size() ;  idx += 1)
		  udp->ports[idx] = pins[idx]->basename();

	    process_udp_table(udp, table, loc);
	    udp->initial  = init;

	    pform_primitives[name] = udp;
      }


	/* Delete the excess tables and lists from the parser. */
      delete parms;
      delete decl;
      delete table;
      delete init_expr;
}

void pform_make_udp(const struct vlltype&loc, perm_string name,
		    bool synchronous_flag, const pform_ident_t&out_name,
		    PExpr*init_expr, list<pform_ident_t>*parms,
		    list<string>*table)
{

      std::vector<PWire*> pins(parms->size() + 1);

	/* Make the PWire for the output port. */
      pins[0] = new PWire(out_name.first, out_name.second,
			  synchronous_flag? NetNet::REG : NetNet::WIRE,
			  NetNet::POUTPUT);
      FILE_NAME(pins[0], loc);

	/* Make the PWire objects for the input ports. */
      { list<pform_ident_t>::iterator cur;
        unsigned idx;
        for (cur = parms->begin(), idx = 1
		   ;  cur != parms->end()
		   ;  idx += 1, ++ cur) {
	      ivl_assert(loc, idx < pins.size());
	      pins[idx] = new PWire(cur->first, cur->second, NetNet::WIRE,
				    NetNet::PINPUT);
	      FILE_NAME(pins[idx], loc);
	}
	ivl_assert(loc, idx == pins.size());
      }

	/* Verify the initial expression, if present, to be sure that
	   it only assigns to the output and the output is
	   registered. Then save the initial value that I get. */
      verinum::V init = verinum::Vx;
      if (init_expr) {
	      // XXXX
	    ivl_assert(*init_expr, pins[0]->get_wire_type() == NetNet::REG);

	    PAssign*pa = dynamic_cast<PAssign*>(init_expr);
	    ivl_assert(*init_expr, pa);

	    const PEIdent*id = dynamic_cast<const PEIdent*>(pa->lval());
	    ivl_assert(*init_expr, id);

	      // XXXX
	      //ivl_assert(*init_expr, id->name() == pins[0]->name());

	    const PENumber*np = dynamic_cast<const PENumber*>(pa->rval());
	    ivl_assert(*init_expr, np);

	    init = np->value()[0];
      }

	// Put the primitive into the primitives table
      if (pform_primitives[name]) {
	    ostringstream msg;
	    msg << "error: Primitive " << name << " was already declared here: "
		<< pform_primitives[name]->get_fileline() << endl;
	      // Some compilers warn if there is just a single C string.
	    VLerror(loc, msg.str().c_str(), "");

      } else {
	    PUdp*udp = new PUdp(name, pins.size());
	    FILE_NAME(udp, loc);

	      // Detect sequential udp.
	    udp->sequential = synchronous_flag;

	      // Make the port list for the UDP
	    for (unsigned idx = 0 ;  idx < pins.size() ;  idx += 1)
		  udp->ports[idx] = pins[idx]->basename();

	    ivl_assert(loc, udp);
	    if (table) {
		  process_udp_table(udp, table, loc);
		  udp->initial  = init;

		  pform_primitives[name] = udp;
	    } else {
		  ostringstream msg;
		  msg << "error: Invalid table for UDP primitive " << name
		      << "." << endl;
		    // Some compilers warn if there is just a single C string.
		  VLerror(loc, msg.str().c_str(), "");
	    }
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
static void pform_set_net_range(PWire *wire,
			        const vector_type_t *vec_type,
				PWSRType rt = SR_NET,
				std::list<named_pexpr_t>*attr = 0)
{
      pform_bind_attributes(wire->attributes, attr, true);

      if (!vec_type)
	    return;

      list<pform_range_t> *range = vec_type->pdims.get();
      if (range)
	    wire->set_range(*range, rt);
      wire->set_signed(vec_type->signed_flag);
}

/*
 * This is invoked to make a named event. This is the declaration of
 * the event, and not necessarily the use of it.
 */
static void pform_make_event(const struct vlltype&loc, const pform_ident_t&name)
{
      PEvent*event = new PEvent(name.first, name.second);
      FILE_NAME(event, loc);

      add_local_symbol(lexical_scope, name.first, event);
      lexical_scope->events[name.first] = event;
}

void pform_make_events(const struct vlltype&loc, const list<pform_ident_t>*names)
{
      for (auto cur = names->begin() ;  cur != names->end() ; ++ cur ) {
	    pform_make_event(loc, *cur);
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
	    cerr << info.get_fileline() << ": error: Gates do not have port names."
		 << endl;
	    error_count += 1;
	    return;
      }

      if (info.parms) {
	    for (list<PExpr*>::iterator cur = info.parms->begin()
		       ; cur != info.parms->end() ; ++cur) {
		  pform_declare_implicit_nets(*cur);
	    }
      }

      perm_string dev_name = lex_strings.make(info.name);
      PGBuiltin*cur = new PGBuiltin(type, dev_name, info.parms, delay);
      cur->set_ranges(info.ranges);

	// The pform_makegates() that calls me will take care of
	// deleting the attr pointer, so tell the
	// pform_bind_attributes function to keep the attr object.
      pform_bind_attributes(cur->attributes, attr, true);

      cur->strength0(str.str0);
      cur->strength1(str.str1);
      cur->set_line(info);

      if (pform_cur_generate) {
	    if (dev_name != "") add_local_symbol(pform_cur_generate, dev_name, cur);
	    pform_cur_generate->add_gate(cur);
      } else {
	    if (dev_name != "") add_local_symbol(pform_cur_module.front(), dev_name, cur);
	    pform_cur_module.front()->add_gate(cur);
      }
}

void pform_makegates(const struct vlltype&loc,
		     PGBuiltin::Type type,
		     struct str_pair_t str,
		     list<PExpr*>*delay,
		     std::vector<lgate>*gates,
		     list<named_pexpr_t>*attr)
{
      ivl_assert(loc, !pform_cur_module.empty());
      if (pform_cur_module.front()->program_block) {
	    cerr << loc << ": error: Gates and switches may not be instantiated in "
		 << "program blocks." << endl;
	    error_count += 1;
      }
      if (pform_cur_module.front()->is_interface) {
	    cerr << loc << ": error: Gates and switches may not be instantiated in "
		 << "interfaces." << endl;
	    error_count += 1;
      }

      for (unsigned idx = 0 ;  idx < gates->size() ;  idx += 1) {
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
			       list<pform_range_t>*ranges,
			       const LineInfo&li,
			       std::list<named_pexpr_t>*attr)
{
      for (list<PExpr*>::iterator idx = wires->begin()
		 ; idx != wires->end() ; ++idx) {
	    pform_declare_implicit_nets(*idx);
      }

      PGModule*cur = new PGModule(type, name, wires);
      cur->set_line(li);
      cur->set_ranges(ranges);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->size();
	    named_pexpr_t *byname = new named_pexpr_t[cnt];

	    std::copy(overrides->by_name->begin(), overrides->by_name->end(),
		      byname);

	    cur->set_parameters(byname, cnt);

      } else if (overrides && overrides->by_order) {
	    cur->set_parameters(overrides->by_order);
      }

      if (pform_cur_generate) {
	    if (name != "") add_local_symbol(pform_cur_generate, name, cur);
	    pform_cur_generate->add_gate(cur);
      } else {
	    if (name != "") add_local_symbol(pform_cur_module.front(), name, cur);
	    pform_cur_module.front()->add_gate(cur);
      }
      pform_bind_attributes(cur->attributes, attr);
}

static void pform_make_modgate(perm_string type,
			       perm_string name,
			       struct parmvalue_t*overrides,
			       list<named_pexpr_t>*bind,
			       list<pform_range_t>*ranges,
			       const LineInfo&li,
			       std::list<named_pexpr_t>*attr)
{
      unsigned npins = bind->size();
      named_pexpr_t *pins = new named_pexpr_t[npins];
      for (const auto &bind_cur : *bind)
            pform_declare_implicit_nets(bind_cur.parm);

      std::copy(bind->begin(), bind->end(), pins);

      PGModule*cur = new PGModule(type, name, pins, npins);
      cur->set_line(li);
      cur->set_ranges(ranges);

      if (overrides && overrides->by_name) {
	    unsigned cnt = overrides->by_name->size();
	    named_pexpr_t *byname = new named_pexpr_t[cnt];

	    std::copy(overrides->by_name->begin(), overrides->by_name->end(),
		      byname);

	    cur->set_parameters(byname, cnt);

      } else if (overrides && overrides->by_order) {

	    cur->set_parameters(overrides->by_order);
      }

      if (pform_cur_generate) {
	    add_local_symbol(pform_cur_generate, name, cur);
	    pform_cur_generate->add_gate(cur);
      } else {
	    add_local_symbol(pform_cur_module.front(), name, cur);
	    pform_cur_module.front()->add_gate(cur);
      }
      pform_bind_attributes(cur->attributes, attr);
}

void pform_make_modgates(const struct vlltype&loc,
			 perm_string type,
			 struct parmvalue_t*overrides,
			 std::vector<lgate>*gates,
			 std::list<named_pexpr_t>*attr)
{
	// The grammer should not allow module gates to happen outside
	// an active module. But if really bad input errors combine in
	// an ugly way with error recovery, then catch this
	// implausible situation and return an error.
      if (pform_cur_module.empty()) {
	    cerr << loc << ": internal error: "
		 << "Module instantiations outside module scope are not possible."
		 << endl;
	    error_count += 1;
	    delete gates;
	    return;
      }
      ivl_assert(loc, !pform_cur_module.empty());

	// Detect some more realistic errors.

      if (pform_cur_module.front()->program_block) {
	    cerr << loc << ": error: Module instantiations are not allowed in "
		 << "program blocks." << endl;
	    error_count += 1;
      }
      if (pform_cur_module.front()->is_interface) {
	    cerr << loc << ": error: Module instantiations are not allowed in "
		 << "interfaces." << endl;
	    error_count += 1;
      }

      for (unsigned idx = 0 ;  idx < gates->size() ;  idx += 1) {
	    lgate cur = (*gates)[idx];
	    perm_string cur_name = lex_strings.make(cur.name);

	    if (cur.parms_by_name) {
		  pform_make_modgate(type, cur_name, overrides,
				     cur.parms_by_name, cur.ranges,
				     cur, attr);

	    } else if (cur.parms) {

		    /* If there are no parameters, the parser will be
		       tricked into thinking it is one empty
		       parameter. This fixes that. */
		  if ((cur.parms->size() == 1) && (cur.parms->front() == 0)) {
			delete cur.parms;
			cur.parms = new list<PExpr*>;
		  }
		  pform_make_modgate(type, cur_name, overrides,
				     cur.parms, cur.ranges,
				     cur, attr);

	    } else {
		  list<PExpr*>*wires = new list<PExpr*>;
		  pform_make_modgate(type, cur_name, overrides,
				     wires, cur.ranges,
				     cur, attr);
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
	    pform_cur_module.front()->add_gate(cur);

      return cur;
}

void pform_make_pgassign_list(const struct vlltype&loc,
			      list<PExpr*>*alist,
			      list<PExpr*>*del,
			      struct str_pair_t str)
{
      ivl_assert(loc, alist->size() % 2 == 0);
      while (! alist->empty()) {
	    PExpr*lval = alist->front(); alist->pop_front();
	    PExpr*rval = alist->front(); alist->pop_front();
	    PGAssign*tmp = pform_make_pgassign(lval, rval, del, str);
	    FILE_NAME(tmp, loc);
      }
}

/*
 * This function makes the initial assignment to a variable as given
 * in the source. It handles the case where a variable is assigned
 * where it is declared, e.g.
 *
 *    reg foo = <expr>;
 *
 * In Verilog-2001 this is only supported at the module level, and is
 * equivalent to the combination of statements:
 *
 *    reg foo;
 *    initial foo = <expr>;
 *
 * In SystemVerilog, variable initializations are allowed in any scope.
 * For static variables, initializations are performed before the start
 * of simulation. For automatic variables, initializations are performed
 * each time the enclosing block is entered. Here we store the variable
 * assignments in the current scope, and later elaboration creates an
 * initialization block that will be executed at the appropriate time.
 *
 * This syntax is not part of the IEEE1364-1995 standard, but is
 * approved by OVI as enhancement BTF-B14.
 */
void pform_make_var_init(const struct vlltype&li, const pform_ident_t&name,
			 PExpr*expr)
{
      if (! pform_at_module_level() && !gn_system_verilog()) {
	    VLerror(li, "error: Variable declaration assignments are only "
                        "allowed at the module level.");
	    delete expr;
	    return;
      }

      PEIdent*lval = new PEIdent(name.first, name.second);
      FILE_NAME(lval, li);
      PAssign*ass = new PAssign(lval, expr, !gn_system_verilog(), true);
      FILE_NAME(ass, li);

      lexical_scope->var_inits.push_back(ass);
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


static PWire* pform_get_or_make_wire(const struct vlltype&li,
				     const pform_ident_t&name,
				     NetNet::Type type,
				     NetNet::PortType ptype,
				     PWSRType rt)
{
      PWire *cur = 0;

	// If this is not a full declaration check if there is already a signal
	// with the same name that can be extended.
      if (rt != SR_BOTH)
	    cur = pform_get_wire_in_scope(name.first);

	// If the wire already exists but isn't yet fully defined,
	// carry on adding details.
      if (rt == SR_NET && cur && !cur->is_net()) {
	      // At the moment there can only be one location for the PWire, if
	      // there is both a port and signal declaration use the location of
	      // the signal.
	    FILE_NAME(cur, li);
	    cur->set_net(type);
	    return cur;
      }

      if (rt == SR_PORT && cur && !cur->is_port()) {
	    cur->set_port(ptype);
	    return cur;
      }

	// If the wire already exists and is fully defined, this
	// must be a redeclaration. Start again with a new wire.
	// The error will be reported when we add the new wire
	// to the scope. Do not delete the old wire - it will
	// remain in the local symbol map.

      cur = new PWire(name.first, name.second, type, ptype, rt);
      FILE_NAME(cur, li);

      pform_put_wire_in_scope(name.first, cur);

      return cur;
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
			      const pform_ident_t&name,
			      NetNet::PortType port_kind,
			      NetNet::Type type,
			      data_type_t*vtype,
			      list<pform_range_t>*urange,
			      list<named_pexpr_t>*attr,
			      bool keep_attr)
{
      pform_check_net_data_type(li, type, vtype);

      PWire *cur = pform_get_or_make_wire(li, name, type, port_kind, SR_BOTH);

      pform_set_net_range(cur, dynamic_cast<vector_type_t*> (vtype), SR_BOTH);

      if (vtype)
	    cur->set_data_type(vtype);

      if (urange) {
	    cur->set_unpacked_idx(*urange);
	    delete urange;
      }

      pform_bind_attributes(cur->attributes, attr, keep_attr);
}

void pform_module_define_port(const struct vlltype&li,
			      list<pform_port_t>*ports,
			      NetNet::PortType port_kind,
			      NetNet::Type type,
			      data_type_t*vtype,
			      list<named_pexpr_t>*attr)
{
      for (list<pform_port_t>::iterator cur = ports->begin()
		 ; cur != ports->end() ; ++ cur ) {

	    data_type_t*use_type = vtype;

	    pform_module_define_port(li, cur->name, port_kind, type, use_type,
				     cur->udims, attr, true);

	    if (cur->expr)
		  pform_make_var_init(li, cur->name, cur->expr);
      }

      delete ports;
      delete attr;
}

/*
 * this is the basic form of pform_makewire. This takes a single simple
 * name, port type, net type, data type, and attributes, and creates
 * the variable/net. Other forms of pform_makewire ultimately call
 * this one to create the wire and stash it.
 */
PWire *pform_makewire(const vlltype&li, const pform_ident_t&name,
		      NetNet::Type type, std::list<pform_range_t> *indices)
{
      PWire*cur = pform_get_or_make_wire(li, name, type, NetNet::NOT_A_PORT, SR_NET);
      ivl_assert(li, cur);

      if (indices && !indices->empty())
	    cur->set_unpacked_idx(*indices);

      return cur;
}

void pform_makewire(const struct vlltype&li,
		    std::list<PExpr*>*delay,
		    str_pair_t str,
		    std::list<decl_assignment_t*>*assign_list,
		    NetNet::Type type,
		    data_type_t*data_type,
		    list<named_pexpr_t>*attr,
		    bool is_const)
{
      if (is_compilation_unit(lexical_scope) && !gn_system_verilog()) {
	    VLerror(li, "error: Variable declarations must be contained within a module.");
	    return;
      }

      std::vector<PWire*> *wires = new std::vector<PWire*>;

      for (list<decl_assignment_t*>::iterator cur = assign_list->begin()
		 ; cur != assign_list->end() ; ++ cur) {
	    decl_assignment_t* curp = *cur;
	    PWire *wire = pform_makewire(li, curp->name, type, &curp->index);
	    wires->push_back(wire);
      }

      pform_set_data_type(li, data_type, wires, type, attr, is_const);

      while (! assign_list->empty()) {
	    decl_assignment_t*first = assign_list->front();
	    assign_list->pop_front();
            if (PExpr*expr = first->expr.release()) {
                  if (type == NetNet::REG || type == NetNet::IMPLICIT_REG) {
                        pform_make_var_init(li, first->name, expr);
                  } else {
		        PEIdent*lval = new PEIdent(first->name.first,
						   first->name.second);
		        FILE_NAME(lval, li);
		        PGAssign*ass = pform_make_pgassign(lval, expr, delay, str);
		        FILE_NAME(ass, li);
                  }
            }
	    delete first;
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
vector<pform_tf_port_t>*pform_make_task_ports(const struct vlltype&loc,
				      NetNet::PortType pt,
				      data_type_t*vtype,
				      list<pform_port_t>*ports,
				      bool allow_implicit)
{
      ivl_assert(loc, pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);
      ivl_assert(loc, ports);

      vector<pform_tf_port_t>*res = new vector<pform_tf_port_t>(0);
      PWSRType rt = SR_BOTH;

      // If this is a non-ansi port declaration and the type is an implicit type
      // this is only a port declaration.
      vector_type_t*vec_type = dynamic_cast<vector_type_t*>(vtype);
      if (allow_implicit && (!vtype || (vec_type && vec_type->implicit_flag)))
	    rt = SR_PORT;

      for (list<pform_port_t>::iterator cur = ports->begin();
	   cur != ports->end(); ++cur) {
	    PWire*curw = pform_get_or_make_wire(loc, cur->name,
						NetNet::IMPLICIT_REG, pt, rt);
	    if (rt == SR_BOTH)
		  curw->set_data_type(vtype);

	    pform_set_net_range(curw, vec_type, rt);

	    if (cur->udims) {
		  if (pform_requires_sv(loc, "Task/function port with unpacked dimensions"))
			curw->set_unpacked_idx(*cur->udims);
	    }

	    res->push_back(pform_tf_port_t(curw));
      }

      delete ports;
      return res;
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

PExpr* pform_genvar_inc_dec(const struct vlltype&loc, const char*name, bool inc_flag)
{
      pform_requires_sv(loc, "Increment/decrement operator");

      PExpr*lval = new PEIdent(lex_strings.make(name), loc.lexical_pos);
      PExpr*rval = new PENumber(new verinum(1));
      FILE_NAME(lval, loc);
      FILE_NAME(rval, loc);

      PEBinary*tmp = new PEBinary(inc_flag ? '+' : '-', lval, rval);
      FILE_NAME(tmp, loc);

      return tmp;
}

PExpr* pform_genvar_compressed(const struct vlltype &loc, const char *name,
			       char op, PExpr *rval)
{
      pform_requires_sv(loc, "Compressed assignment operator");

      PExpr *lval = new PEIdent(lex_strings.make(name), loc.lexical_pos);
      FILE_NAME(lval, loc);

      PExpr *expr;
      switch (op) {
	  case 'l':
	  case 'r':
	  case 'R':
	    expr = new PEBShift(op, lval, rval);
	    break;
	  default:
	    expr = new PEBinary(op, lval, rval);
	    break;
      }
      FILE_NAME(expr, loc);

      return expr;
}

void pform_set_attrib(perm_string name, perm_string key, char*value)
{
      if (PWire*cur = lexical_scope->wires_find(name)) {
	    cur->attributes[key] = new PEString(value);

      } else if (PGate*curg = pform_cur_module.front()->get_gate(name)) {
	    curg->attributes[key] = new PEString(value);

      } else {
	    delete[] value;
	    VLerror("error: Unable to match name for setting attribute.");

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
	    VLerror("error: Type name is not (yet) defined.");
	    delete[] value;
	    return;
      }

      (*udp).second ->attributes[key] = new PEString(value);
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

static void pform_set_type_parameter(const struct vlltype&loc, perm_string name,
				     const LexicalScope::range_t*value_range)
{
      pform_requires_sv(loc, "Type parameter");

      if (value_range)
	    VLerror(loc, "error: Type parameter must not have value range.");

      type_parameter_t *type = new type_parameter_t(name);
      pform_set_typedef(loc, name, type, 0);
}

void pform_set_parameter(const struct vlltype&loc,
			 perm_string name, bool is_local, bool is_type,
			 data_type_t*data_type, list<pform_range_t>*udims,
			 PExpr*expr, LexicalScope::range_t*value_range)
{
      LexicalScope*scope = lexical_scope;
      if (is_compilation_unit(scope) && !gn_system_verilog()) {
	    VLerror(loc, "error: %s declarations must be contained within a module.",
		         is_local ? "localparam" : "parameter");
	    return;
      }

      if (expr == 0) {
	    if (is_local) {
		  VLerror(loc, "error: localparam must have a value.");
	    } else if (!pform_in_parameter_port_list) {
		  VLerror(loc, "error: parameter declared outside parameter "
			        "port list must have a default value.");
	    } else {
		  pform_requires_sv(loc, "parameter without default value");
	    }
      }

      vector_type_t*vt = dynamic_cast<vector_type_t*>(data_type);
      if (vt && vt->pdims && vt->pdims->size() > 1) {
	    if (pform_requires_sv(loc, "packed array parameter")) {
		  VLerror(loc, "sorry: packed array parameters are not supported yet.");
	    }
	    return;
      }

      if (udims) {
	    if (pform_requires_sv(loc, "unpacked array parameter")) {
		  VLerror(loc, "sorry: unpacked array parameters are not supported yet.");
	    }
	    return;
      }

      bool overridable = !is_local;

      if (scope == pform_cur_generate && !is_local) {
	    if (!gn_system_verilog()) {
		  VLerror(loc, "parameter declarations are not permitted in generate blocks");
		  return;
	    }
	    // SystemVerilog allows `parameter` in generate blocks, but it has
	    // the same semantics as `localparam` in that scope.
	    overridable = false;
      }

      bool in_module = dynamic_cast<Module*>(scope) &&
		       scope == pform_cur_module.front();

      if (!pform_in_parameter_port_list && in_module &&
          scope->has_parameter_port_list)
	    overridable = false;

      if (pform_in_class())
	    overridable = false;

      Module::param_expr_t*parm = new Module::param_expr_t();
      FILE_NAME(parm, loc);

      if (is_type)
	    pform_set_type_parameter(loc, name, value_range);
      else
	    add_local_symbol(scope, name, parm);

      parm->expr = expr;
      parm->data_type = data_type;
      parm->range = value_range;
      parm->local_flag = is_local;
      parm->overridable = overridable;
      parm->type_flag = is_type;
      parm->lexical_pos = loc.lexical_pos;

      scope->parameters[name] = parm;

      // Only a module keeps the position of the parameter.
      if (overridable && in_module)
	    pform_cur_module.front()->param_names.push_back(name);
}

void pform_set_specparam(const struct vlltype&loc, perm_string name,
			 list<pform_range_t>*range, PExpr*expr)
{
      ivl_assert(loc, !pform_cur_module.empty());
      Module*scope = pform_cur_module.front();
      if (scope != lexical_scope) {
	    delete range;
	    delete expr;
	    return;
      }

      ivl_assert(loc, expr);
      Module::param_expr_t*parm = new Module::param_expr_t();
      FILE_NAME(parm, loc);

      add_local_symbol(scope, name, parm);
      pform_cur_module.front()->specparams[name] = parm;

      parm->expr = expr;
      parm->range = 0;

      if (range) {
	    ivl_assert(loc, range->size() == 1);
	    parm->data_type = new vector_type_t(IVL_VT_LOGIC, false, range);
	    parm->range = 0;
      }
}

void pform_set_defparam(const pform_name_t&name, PExpr*expr)
{
      assert(expr);
      if (pform_cur_generate)
            pform_cur_generate->defparms.push_back(make_pair(name,expr));
      else
            pform_cur_module.front()->defparms.push_back(make_pair(name,expr));
}

void pform_make_let(const struct vlltype&loc,
                    perm_string name,
                    list<PLet::let_port*>*ports,
                    PExpr*expr)
{
      LexicalScope*scope =  pform_peek_scope();

      cerr << loc.get_fileline() << ": sorry: let declarations ("
           << name << ") are not currently supported." << endl;
      error_count += 1;

      PLet*res = new PLet(name, scope, ports, expr);
      FILE_NAME(res, loc);

/*
      cerr << "Found: ";
      res->dump(cerr, 0);
*/

      delete res;
      delete ports;
      delete expr;
}

PLet::let_port_t* pform_make_let_port(data_type_t*data_type,
                                      perm_string name,
                                      list<pform_range_t>*range,
                                      PExpr*def)
{
      PLet::let_port_t*res = new PLet::let_port_t;

      res->type_ = data_type;
      res->name_ = name;
      res->range_ = range;
      res->def_ = def;

      return res;
}

/*
 * Specify paths.
 */
extern PSpecPath* pform_make_specify_path(const struct vlltype&li,
					  list<perm_string>*src, char pol,
					  bool full_flag, list<perm_string>*dst)
{
      PSpecPath*path = new PSpecPath(*src, *dst, pol, full_flag);
      FILE_NAME(path, li);

      delete src;
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

      ivl_assert(*path, path->delays.empty());

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
      pform_cur_module.front()->specify_paths.push_back(obj);
}

/*
 * Timing checks.
 */
 extern PRecRem* pform_make_recrem(const struct vlltype&li,
			 PTimingCheck::event_t*reference_event,
			 PTimingCheck::event_t*data_event,
			 PExpr*setup_limit,
			 PExpr*hold_limit,
			 PTimingCheck::optional_args_t* args)
{
      ivl_assert(li, args);

      PRecRem*recrem = new PRecRem(
	      reference_event,
	      data_event,
	      setup_limit,
	      hold_limit,
	      args->notifier,
	      args->timestamp_cond,
	      args->timecheck_cond,
	      args->delayed_reference,
	      args->delayed_data
      );

      FILE_NAME(recrem, li);

      return recrem;
}
extern PSetupHold* pform_make_setuphold(const struct vlltype&li,
			 PTimingCheck::event_t*reference_event,
			 PTimingCheck::event_t*data_event,
			 PExpr*setup_limit,
			 PExpr*hold_limit,
			 PTimingCheck::optional_args_t* args)
{
      ivl_assert(li, args);

      PSetupHold*setuphold = new PSetupHold(
	      reference_event,
	      data_event,
	      setup_limit,
	      hold_limit,
	      args->notifier,
	      args->timestamp_cond,
	      args->timecheck_cond,
	      args->delayed_reference,
	      args->delayed_data
      );

      FILE_NAME(setuphold, li);

      return setuphold;
}

extern void pform_module_timing_check(PTimingCheck*obj)
{
      if (!obj)
	    return;

      pform_cur_module.front()->timing_checks.push_back(obj);
}


void pform_set_port_type(const struct vlltype&li,
			 list<pform_port_t>*ports,
			 NetNet::PortType pt,
			 data_type_t*dt,
			 list<named_pexpr_t>*attr)
{
      ivl_assert(li, pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);

      vector_type_t *vt = dynamic_cast<vector_type_t*> (dt);

      bool have_init_expr = false;
      for (list<pform_port_t>::iterator cur = ports->begin()
		 ; cur != ports->end() ; ++ cur ) {

	    PWire *wire = pform_get_or_make_wire(li, cur->name,
						 NetNet::IMPLICIT, pt, SR_PORT);
	    pform_set_net_range(wire, vt, SR_PORT, attr);

	    if (cur->udims) {
		  cerr << li << ": warning: "
		       << "Array dimensions in incomplete port declarations "
		       << "are currently ignored." << endl;
		  cerr << li << ":        : "
		       << "The dimensions specified in the net or variable "
		       << "declaration will be used." << endl;
		  delete cur->udims;
	    }
	    if (cur->expr) {
		  have_init_expr = true;
		  delete cur->expr;
	    }
      }
      if (have_init_expr) {
	    cerr << li << ": error: "
		 << "Incomplete port declarations cannot be initialized."
		 << endl;
	    error_count += 1;
      }

      delete ports;
      delete dt;
      delete attr;
}

/*
 * This function detects the derived class for the given type and
 * dispatches the type to the proper subtype function.
 */
void pform_set_data_type(const struct vlltype&li, data_type_t*data_type,
			 std::vector<PWire*> *wires, NetNet::Type net_type,
			 list<named_pexpr_t>*attr, bool is_const)
{
      if (data_type == 0) {
	    VLerror(li, "internal error: data_type==0.");
	    ivl_assert(li, 0);
      }

      vector_type_t*vec_type = dynamic_cast<vector_type_t*> (data_type);

      for (std::vector<PWire*>::iterator it= wires->begin();
	   it != wires->end() ; ++it) {
	    PWire *wire = *it;

	    pform_set_net_range(wire, vec_type);

	    // If these fail there is a bug somewhere else. pform_set_data_type()
	    // is only ever called on a fresh wire that already exists.
	    bool rc = wire->set_wire_type(net_type);
	    ivl_assert(li, rc);

	    wire->set_data_type(data_type);
	    wire->set_const(is_const);

	    pform_bind_attributes(wire->attributes, attr, true);
      }

      delete wires;
}

vector<PWire*>* pform_make_udp_input_ports(list<pform_ident_t>*names)
{
      vector<PWire*>*out = new vector<PWire*>(names->size());

      unsigned idx = 0;
      for (list<pform_ident_t>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    PWire*pp = new PWire(cur->first, cur->second,
				 NetNet::IMPLICIT,
				 NetNet::PINPUT);
	    (*out)[idx] = pp;
	    idx += 1;
      }

      delete names;
      return out;
}

PProcess* pform_make_behavior(ivl_process_type_t type, Statement*st,
			      list<named_pexpr_t>*attr)
{
	// Add an implicit @* around the statement for the always_comb and
	// always_latch statements.
      if ((type == IVL_PR_ALWAYS_COMB) || (type == IVL_PR_ALWAYS_LATCH)) {
	    PEventStatement *tmp = new PEventStatement(true);
	    tmp->set_line(*st);
	    tmp->set_statement(st);
	    st = tmp;
      }

      PProcess*pp = new PProcess(type, st);

	// If we are in a part of the code where the meta-comment
	// synthesis translate_off is in effect, then implicitly add
	// the ivl_synthesis_off attribute to any behavioral code that
	// we run into.
      if (pform_mc_translate_flag == false) {
	    if (attr == 0) attr = new list<named_pexpr_t>;
	    named_pexpr_t tmp;
	    tmp.name = perm_string::literal("ivl_synthesis_off");
	    tmp.parm = 0;
	    attr->push_back(tmp);
      }

      pform_bind_attributes(pp->attributes, attr);

      pform_put_behavior_in_scope(pp);

      ivl_assert(*st, ! pform_cur_module.empty());
      if (pform_cur_module.front()->program_block &&
          ((type == IVL_PR_ALWAYS) || (type == IVL_PR_ALWAYS_COMB) ||
           (type == IVL_PR_ALWAYS_FF) || (type == IVL_PR_ALWAYS_LATCH))) {
	    cerr << st->get_fileline() << ": error: Always statements are not allowed"
		 << " in program blocks." << endl;
	    error_count += 1;
      }

      return pp;
}

void pform_start_modport_item(const struct vlltype&loc, const char*name)
{
      Module*scope = pform_cur_module.front();
      ivl_assert(loc, scope && scope->is_interface);
      ivl_assert(loc, pform_cur_modport == 0);

      perm_string use_name = lex_strings.make(name);
      pform_cur_modport = new PModport(use_name);
      FILE_NAME(pform_cur_modport, loc);

      add_local_symbol(scope, use_name, pform_cur_modport);
      scope->modports[use_name] = pform_cur_modport;

      delete[] name;
}

void pform_end_modport_item(const struct vlltype&loc)
{
      ivl_assert(loc, pform_cur_modport);
      pform_cur_modport = 0;
}

void pform_add_modport_port(const struct vlltype&loc,
                            NetNet::PortType port_type,
                            perm_string name, PExpr*expr)
{
      ivl_assert(loc, pform_cur_modport);

      if (pform_cur_modport->simple_ports.find(name)
	  != pform_cur_modport->simple_ports.end()) {
	    cerr << loc << ": error: duplicate declaration of port '"
		 << name << "' in modport list '"
		 << pform_cur_modport->name() << "'." << endl;
	    error_count += 1;
      }
      pform_cur_modport->simple_ports[name] = make_pair(port_type, expr);
}

bool pform_requires_sv(const struct vlltype&loc, const char *feature)
{
      if (gn_system_verilog())
	    return true;

      VLerror(loc, "error: %s requires SystemVerilog.", feature);

      return false;
}

void pform_block_decls_requires_sv(void)
{
      for (auto const& wire : lexical_scope->wires) {
	    struct vlltype loc;
	    loc.text = wire.second->get_file();
	    loc.first_line = wire.second->get_lineno();
	    pform_requires_sv(loc, "Variable declaration in unnamed block");
      }
}

void pform_check_net_data_type(const struct vlltype&loc, NetNet::Type net_type,
			       const data_type_t *data_type)
{
      // For SystemVerilog the type is checked during elaboration since due to
      // forward typedefs and type parameters the actual type might not be known
      // yet.
      if (gn_system_verilog())
	    return;

      switch (net_type) {
      case NetNet::REG:
      case NetNet::IMPLICIT_REG:
	    return;
      default:
	    break;
      }

      if (!data_type)
	    return;

      const vector_type_t*vec_type = dynamic_cast<const vector_type_t*>(data_type);
      if (vec_type && vec_type->implicit_flag)
	    return;

      if (!gn_cadence_types_flag)
	    VLerror(loc, "Net data type requires SystemVerilog or -gxtypes.");

      if (vec_type)
	    return;

      const real_type_t*rtype = dynamic_cast<const real_type_t*>(data_type);
      if (rtype && rtype->type_code() == real_type_t::REAL)
	    return;

      pform_requires_sv(loc, "Net data type");
}

FILE*vl_input = 0;
extern void reset_lexor();

int pform_parse(const char*path)
{
      vl_file = path;
      if (strcmp(path, "-") == 0) {
	    vl_input = stdin;
      } else if (ivlpp_string) {
	    char*cmdline = (char*)malloc(strlen(ivlpp_string) +
					        strlen(path) + 4);
	    strcpy(cmdline, ivlpp_string);
	    strcat(cmdline, " \"");
	    strcat(cmdline, path);
	    strcat(cmdline, "\"");

	    if (verbose_flag)
		  cerr << "Executing: " << cmdline << endl<< flush;

	    vl_input = popen(cmdline, "r");
	    if (vl_input == 0) {
		  cerr << "Unable to preprocess " << path << "." << endl;
		  return 1;
	    }

	    if (verbose_flag)
		  cerr << "...parsing output from preprocessor..." << endl << flush;

	    free(cmdline);
      } else {
	    vl_input = fopen(path, "r");
	    if (vl_input == 0) {
		  cerr << "Unable to open " << path << "." << endl;
		  return 1;
	    }
      }

      if (pform_units.empty() || separate_compilation) {
	    char unit_name[20];
	    static unsigned nunits = 0;
	    if (separate_compilation)
		  snprintf(unit_name, sizeof(unit_name)-1, "$unit#%u", ++nunits);
	    else
		  snprintf(unit_name, sizeof(unit_name)-1, "$unit");

	    PPackage*unit = new PPackage(lex_strings.make(unit_name), 0);
	    unit->default_lifetime = LexicalScope::STATIC;
	    unit->set_file(filename_strings.make(path));
	    unit->set_lineno(1);
	    pform_units.push_back(unit);

            pform_cur_module.clear();
            pform_cur_generate = 0;
            pform_cur_modport = 0;

	    pform_set_timescale(def_ts_units, def_ts_prec, 0, 0);

	    allow_timeunit_decl = true;
	    allow_timeprec_decl = true;

	    lexical_scope = unit;
      }
      reset_lexor();
      error_count = 0;
      warn_count = 0;
      int rc = VLparse();

      if (vl_input != stdin) {
	    if (ivlpp_string)
		  pclose(vl_input);
	    else
		  fclose(vl_input);
      }

      if (rc) {
	    cerr << "I give up." << endl;
	    error_count += 1;
      }

      destroy_lexor();
      return error_count;
}

void pform_finish()
{
      // Wait until all parsing is done and all symbols in the unit scope are
      // known before importing possible imports.
      for (auto unit : pform_units)
	    pform_check_possible_imports(unit);
}
