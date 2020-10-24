/*
 * Copyright (c) 1998-2020 Stephen Williams (steve@icarus.com)
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

  /* increment this for generate schemes within a module, and set it
     to zero when a new module starts. */
static unsigned scope_generate_counter = 1;

  /* This tracks the current generate scheme being processed. This is
     always within a module. */
static PGenerate*pform_cur_generate = 0;

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

LexicalScope* pform_peek_scope(void)
{
      assert(lexical_scope);
      return lexical_scope;
}

void pform_pop_scope()
{
      LexicalScope*scope = lexical_scope;
      assert(scope);

      map<perm_string,PPackage*>::const_iterator cur;
      for (cur = scope->possible_imports.begin(); cur != scope->possible_imports.end(); ++cur) {
            if (scope->local_symbols.find(cur->first) == scope->local_symbols.end())
                  scope->explicit_imports[cur->first] = cur->second;
      }
      scope->possible_imports.clear();

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

static PPackage*find_potential_import(const struct vlltype&loc, LexicalScope*scope,
				      perm_string name, bool tf_call, bool make_explicit)
{
      assert(scope);

      PPackage*found_pkg = 0;
      for (set<PPackage*>::const_iterator cur_pkg = scope->potential_imports.begin();
	      cur_pkg != scope->potential_imports.end(); ++cur_pkg) {
	    PPackage*search_pkg = *cur_pkg;
	    map<perm_string,PNamedItem*>::const_iterator cur_sym
		= search_pkg->local_symbols.find(name);
	    if (cur_sym != search_pkg->local_symbols.end()) {
		  if (found_pkg && make_explicit) {
			cerr << loc.get_fileline() << ": error: "
				"Ambiguous use of '" << name << "'. "
				"It is exported by both '"
			      << found_pkg->pscope_name()
			      << "' and by '"
			      << search_pkg->pscope_name()
			      << "'." << endl;
			error_count += 1;
		  } else {
			found_pkg = search_pkg;
			if (make_explicit) {
                              if (tf_call)
			            scope->possible_imports[name] = found_pkg;
                              else
			            scope->explicit_imports[name] = found_pkg;
                        }
		  }
	    }
      }
      return found_pkg;
}

static void check_potential_imports(const struct vlltype&loc, perm_string name, bool tf_call)
{
      LexicalScope*scope = lexical_scope;
      while (scope) {
	    if (scope->local_symbols.find(name) != scope->local_symbols.end())
		  return;
	    if (scope->explicit_imports.find(name) != scope->explicit_imports.end())
		  return;
	    if (find_potential_import(loc, scope, name, tf_call, true))
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
      assert(scope);

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
		  VLerror("error: a timeprecision is missing or is too large!");
	    }
      } else {
            assert(scope->time_unit >= scope->time_precision);
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

PClass* pform_push_class_scope(const struct vlltype&loc, perm_string name,
			       LexicalScope::lifetime_t lifetime)
{
      PClass*class_scope = new PClass(name, lexical_scope);
      class_scope->default_lifetime = find_lifetime(lifetime);
      FILE_NAME(class_scope, loc);

      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      assert(scopex);
      assert(!pform_cur_generate);

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
      assert(scopex);
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
      assert(scopex);
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

      return new PEIdent(name);
}

PTrigger* pform_new_trigger(const struct vlltype&loc, PPackage*pkg,
			    const pform_name_t&name)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, false);

      PTrigger*tmp = new PTrigger(pkg, name);
      FILE_NAME(tmp, loc);
      return tmp;
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

static void pform_put_enum_type_in_scope(enum_type_t*enum_set)
{
      if (lexical_scope->enum_sets.count(enum_set))
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

      lexical_scope->enum_sets.insert(enum_set);
}

PWire*pform_get_make_wire_in_scope(const struct vlltype&, perm_string name,
                                   NetNet::Type net_type, NetNet::PortType port_type,
                                   ivl_variable_type_t vt_type)
{
      PWire*cur = pform_get_wire_in_scope(name);

	// If the wire already exists and is fully defined, this
	// must be a redeclaration. Start again with a new wire.
	// The error will be reported when we add the new wire
	// to the scope. Do not delete the old wire - it will
	// remain in the local symbol map.
      if (cur && cur->get_data_type() != IVL_VT_NO_TYPE)
            cur = 0;

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

void pform_set_typedef(perm_string name, data_type_t*data_type, std::list<pform_range_t>*unp_ranges)
{
      if(unp_ranges)
	    data_type = new uarray_type_t(data_type, unp_ranges);

      add_local_symbol(lexical_scope, name, data_type);

      data_type_t*&ref = lexical_scope->typedefs[name];

      ivl_assert(*data_type, ref == 0);
      ref = data_type;
      ref->name = name;

      if (enum_type_t*enum_type = dynamic_cast<enum_type_t*>(data_type))
	    pform_put_enum_type_in_scope(enum_type);
}

void pform_set_type_referenced(const struct vlltype&loc, const char*name)
{
      perm_string lex_name = lex_strings.make(name);
      check_potential_imports(loc, lex_name, false);
}

data_type_t* pform_test_type_identifier(const struct vlltype&loc, const char*txt)
{
      perm_string name = lex_strings.make(txt);

      LexicalScope*cur_scope = lexical_scope;
      do {
	    map<perm_string,data_type_t*>::iterator cur;

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

            PPackage*pkg = find_potential_import(loc, cur_scope, name, false, false);
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

/*
 * The parser uses this function to test if the name is a typedef in
 * the current scope. We use this to know if we can override the
 * definition because it shadows a containing scope.
 */
bool pform_test_type_identifier_local(perm_string name)
{
      LexicalScope*cur_scope = lexical_scope;

      map<perm_string,data_type_t*>::iterator cur;

      cur = cur_scope->typedefs.find(name);
      if (cur != cur_scope->typedefs.end())
	    return true;

      return false;
}

PECallFunction* pform_make_call_function(const struct vlltype&loc,
					 const pform_name_t&name,
					 const list<PExpr*>&parms)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, true);

      PECallFunction*tmp = new PECallFunction(name, parms);
      FILE_NAME(tmp, loc);
      return tmp;
}

PCallTask* pform_make_call_task(const struct vlltype&loc,
				const pform_name_t&name,
				const list<PExpr*>&parms)
{
      if (gn_system_verilog())
	    check_potential_imports(loc, name.front().name, true);

      PCallTask*tmp = new PCallTask(name, parms);
      FILE_NAME(tmp, loc);
      return tmp;
}

void pform_make_foreach_declarations(const struct vlltype&loc,
				     std::list<perm_string>*loop_vars)
{
      static const struct str_pair_t str = { IVL_DR_STRONG, IVL_DR_STRONG };

      list<decl_assignment_t*>assign_list;
      for (list<perm_string>::const_iterator cur = loop_vars->begin()
		 ; cur != loop_vars->end() ; ++ cur) {
	    decl_assignment_t*tmp_assign = new decl_assignment_t;
	    tmp_assign->name = lex_strings.make(*cur);
	    assign_list.push_back(tmp_assign);
      }

      pform_makewire(loc, 0, str, &assign_list, NetNet::REG, &size_type);
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

void pform_set_timeunit(const char*txt, bool initial_decl)
{
      int val;

      if (get_time_unit_prec(txt, val, true)) return;

      PScopeExtra*scope = dynamic_cast<PScopeExtra*>(lexical_scope);
      assert(scope);

      if (initial_decl) {
            scope->time_unit = val;
            scope->time_unit_is_local = true;
            scope->time_unit_is_default = false;
            allow_timeunit_decl = false;
      } else if (!scope->time_unit_is_local) {
            VLerror(yylloc, "error: repeat timeunit found and the initial "
                            "timeunit for this scope is missing.");
      } else if (scope->time_unit != val) {
            VLerror(yylloc, "error: repeat timeunit does not match the "
                            "initial timeunit for this scope.");
      }
}

int pform_get_timeunit()
{
      PScopeExtra*scopex = find_nearest_scopex(lexical_scope);
      assert(scopex);
      return scopex->time_unit;
}

void pform_set_timeprec(const char*txt, bool initial_decl)
{
      int val;

      if (get_time_unit_prec(txt, val, false)) return;

      PScopeExtra*scope = dynamic_cast<PScopeExtra*>(lexical_scope);
      assert(scope);

      if (initial_decl) {
            scope->time_precision = val;
            scope->time_prec_is_local = true;
            scope->time_prec_is_default = false;
            allow_timeprec_decl = false;
      } else if (!scope->time_prec_is_local) {
            VLerror(yylloc, "error: repeat timeprecision found and the initial "
                            "timeprecision for this scope is missing.");
      } else if (scope->time_precision != val) {
            VLerror(yylloc, "error: repeat timeprecision does not match the "
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

      if (lifetime != LexicalScope::INHERITED && !gn_system_verilog()) {
	    cerr << loc << ": error: Default subroutine lifetimes "
		    "require SystemVerilog." << endl;
	    error_count += 1;
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

      add_local_symbol(lexical_scope, lex_name, cur_module);

      lexical_scope = cur_module;

	/* The generate scheme numbering starts with *1*, not
	   zero. That's just the way it is, thanks to the standard. */
      scope_generate_counter = 1;

      pform_bind_attributes(cur_module->attributes, attr);
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
	    while (pform_cur_module.size() > 0) {
		  Module*tmp_module = pform_cur_module.front();
		  perm_string tmp_name = tmp_module->mod_name();
		  pform_cur_module.pop_front();
		  ostringstream msg;
		  msg << "Module " << mod_name
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
	    msg << "Module " << name << " was already declared here: "
		<< test->second->get_fileline() << endl;
	    VLerror(msg.str().c_str());
      } else {
	    use_module_map[mod_name] = cur_module;
      }

	// The current lexical scope should be this module by now.
      ivl_assert(*cur_module, lexical_scope == cur_module);
      pform_pop_scope();
}

void pform_genvars(const struct vlltype&li, list<perm_string>*names)
{
      list<perm_string>::const_iterator cur;
      for (cur = names->begin(); cur != names->end() ; *cur++) {
	    PGenvar*genvar = new PGenvar();
	    FILE_NAME(genvar, li);

	    if (pform_cur_generate) {
		  add_local_symbol(pform_cur_generate, *cur, genvar);
		  pform_cur_generate->genvars[*cur] = genvar;
	    } else {
		  add_local_symbol(pform_cur_module.front(), *cur, genvar);
		  pform_cur_module.front()->genvars[*cur] = genvar;
	    }
      }

      delete names;
}

void pform_start_generate_for(const struct vlltype&li,
			      bool local_index,
			      char*ident1, PExpr*init,
			      PExpr*test,
			      char*ident2, PExpr*next)
{
      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
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
      PGenerate*gen = new PGenerate(lexical_scope, scope_generate_counter++);
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
      assert(pform_cur_generate);
      assert(pform_cur_generate->scheme_type == PGenerate::GS_CONDIT);

      PGenerate*cur = pform_cur_generate;
      pform_endgenerate(false);

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

      conditional_block_names.push_front(set<perm_string>());
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
		    vector<PWire*>*decl, list<string>*table,
		    Statement*init_expr,
		    const char*file, unsigned lineno)
{
      unsigned local_errors = 0;
      assert(!parms->empty());

      assert(decl);

	/* Put the declarations into a map, so that I can check them
	   off with the parameters in the list. If the port is already
	   in the map, merge the port type. I will rebuild a list
	   of parameters for the PUdp object. */
      map<perm_string,PWire*> defs;
      for (unsigned idx = 0 ;  idx < decl->size() ;  idx += 1) {

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
		   ; ++ idx, ++ cur) {
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
	    VLwarn("UDP primitive already exists.");

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
		   ;  idx += 1, ++ cur) {
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

/*
 * This function attaches a range to a given name. The function is
 * only called by the parser within the scope of the net declaration,
 * and the name that I receive only has the tail component.
 */
static void pform_set_net_range(perm_string name,
				NetNet::Type net_type,
				const list<pform_range_t>*range,
				bool signed_flag,
				ivl_variable_type_t dt,
				PWSRType rt,
				std::list<named_pexpr_t>*attr)
{
      PWire*cur = pform_get_wire_in_scope(name);
      if (cur == 0) {
	    VLerror("error: name is not a valid net.");
	    return;
      }

	// If this is not implicit ("implicit" meaning we don't
	// know what the type is yet) then set the type now.
      if (net_type != NetNet::IMPLICIT && net_type != NetNet::NONE) {
	    bool rc = cur->set_wire_type(net_type);
	    if (rc == false) {
		  ostringstream msg;
		  msg << name << " " << net_type
		      << " definition conflicts with " << cur->get_wire_type()
		      << " definition at " << cur->get_fileline()
		      << ".";
		  VLerror(msg.str().c_str());
	    }
      }

      if (range == 0) {
	      /* This is the special case that we really mean a
		 scalar. Set a fake range. */
	    cur->set_range_scalar(rt);

      } else {
	    cur->set_range(*range, rt);
      }
      cur->set_signed(signed_flag);

      if (dt != IVL_VT_NO_TYPE)
	    cur->set_data_type(dt);

      pform_bind_attributes(cur->attributes, attr, true);
}

static void pform_set_net_range(list<perm_string>*names,
				list<pform_range_t>*range,
				bool signed_flag,
				ivl_variable_type_t dt,
				NetNet::Type net_type,
				std::list<named_pexpr_t>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_net_range(txt, net_type, range, signed_flag, dt, SR_NET, attr);
      }

}

/*
 * This is invoked to make a named event. This is the declaration of
 * the event, and not necessarily the use of it.
 */
static void pform_make_event(perm_string name, const char*fn, unsigned ln)
{
      PEvent*event = new PEvent(name);
      FILE_NAME(event, fn, ln);

      add_local_symbol(lexical_scope, name, event);
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

      if (info.parms) {
	    for (list<PExpr*>::iterator cur = info.parms->begin()
		       ; cur != info.parms->end() ; ++cur) {
		  pform_declare_implicit_nets(*cur);
	    }
      }

      perm_string dev_name = lex_strings.make(info.name);
      PGBuiltin*cur = new PGBuiltin(type, dev_name, info.parms, delay);
      if (info.range.first)
	    cur->set_range(info.range.first, info.range.second);

	// The pform_makegates() that calls me will take care of
	// deleting the attr pointer, so tell the
	// pform_bind_attributes function to keep the attr object.
      pform_bind_attributes(cur->attributes, attr, true);

      cur->strength0(str.str0);
      cur->strength1(str.str1);
      FILE_NAME(cur, info.file, info.lineno);

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
		     svector<lgate>*gates,
		     list<named_pexpr_t>*attr)
{
      assert(! pform_cur_module.empty());
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
			       const char*fn, unsigned ln,
			       std::list<named_pexpr_t>*attr)
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
			       PExpr*msb, PExpr*lsb,
			       const char*fn, unsigned ln,
			       std::list<named_pexpr_t>*attr)
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
			 svector<lgate>*gates,
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
      assert(! pform_cur_module.empty());

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

      for (unsigned idx = 0 ;  idx < gates->count() ;  idx += 1) {
	    lgate cur = (*gates)[idx];
	    perm_string cur_name = lex_strings.make(cur.name);

	    if (cur.parms_by_name) {
		  pform_make_modgate(type, cur_name, overrides,
				     cur.parms_by_name,
				     cur.range.first, cur.range.second,
				     cur.file, cur.lineno, attr);

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
				     cur.range.first, cur.range.second,
				     cur.file, cur.lineno, attr);

	    } else {
		  list<PExpr*>*wires = new list<PExpr*>;
		  pform_make_modgate(type, cur_name, overrides,
				     wires,
				     cur.range.first, cur.range.second,
				     cur.file, cur.lineno, attr);
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
void pform_make_var_init(const struct vlltype&li,
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
	    VLerror(li, "internal error: var_init to non-register?");
	    delete expr;
	    return;
      }

      PEIdent*lval = new PEIdent(name);
      FILE_NAME(lval, li);
      PAssign*ass = new PAssign(lval, expr, !gn_system_verilog());
      FILE_NAME(ass, li);

      lexical_scope->var_inits.push_back(ass);
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
			      NetNet::PortType port_kind,
			      NetNet::Type type,
			      data_type_t*vtype,
			      list<named_pexpr_t>*attr,
			      bool keep_attr)
{
      struct_type_t*struct_type = 0;
      ivl_variable_type_t data_type = IVL_VT_NO_TYPE;
      bool signed_flag = false;

      PWire*cur = pform_get_wire_in_scope(name);
      if (cur) {
	    ostringstream msg;
	    msg << name << " definition conflicts with "
		<< "definition at " << cur->get_fileline()
		<< ".";
	    VLerror(msg.str().c_str());
	    return;
      }

	// Packed ranges
      list<pform_range_t>*prange = 0;
	// Unpacked dimensions
      list<pform_range_t>*urange = 0;

	// If this is an unpacked array, then split out the parts that
	// we can send to the PWire object that we create.
      if (uarray_type_t*uarr_type = dynamic_cast<uarray_type_t*> (vtype)) {
	    urange = uarr_type->dims.get();
	    vtype = uarr_type->base_type;
      }

      if (vector_type_t*vec_type = dynamic_cast<vector_type_t*> (vtype)) {
	    data_type = vec_type->base_type;
	    signed_flag = vec_type->signed_flag;
	    prange = vec_type->pdims.get();
	    if (vec_type->reg_flag)
		  type = NetNet::REG;

      } else if (atom2_type_t*atype = dynamic_cast<atom2_type_t*>(vtype)) {
	    data_type = IVL_VT_BOOL;
	    signed_flag = atype->signed_flag;
	    prange = make_range_from_width(atype->type_code);

      } else if (real_type_t*rtype = dynamic_cast<real_type_t*>(vtype)) {
	    data_type = IVL_VT_REAL;
	    signed_flag = true;
	    prange = 0;

	    if (rtype->type_code != real_type_t::REAL) {
		  VLerror(li, "sorry: Only real (not shortreal) supported here (%s:%d).",
			  __FILE__, __LINE__);
	    }

      } else if ((struct_type = dynamic_cast<struct_type_t*>(vtype))) {
	    data_type = struct_type->figure_packed_base_type();
	    signed_flag = false;
	    prange = 0;

      } else if (enum_type_t*enum_type = dynamic_cast<enum_type_t*>(vtype)) {
	    data_type = enum_type->base_type;
	    signed_flag = enum_type->signed_flag;
	    prange = enum_type->range.get();

      } else if (vtype) {
	    VLerror(li, "sorry: Given type %s not supported here (%s:%d).",
		    typeid(*vtype).name(), __FILE__, __LINE__);
      }


	// The default type for all flavor of ports is LOGIC.
      if (data_type == IVL_VT_NO_TYPE)
	    data_type = IVL_VT_LOGIC;

      cur = new PWire(name, type, port_kind, data_type);
      FILE_NAME(cur, li);

      cur->set_signed(signed_flag);

      if (struct_type) {
	    cur->set_data_type(struct_type);

      } else if (prange == 0) {
	    cur->set_range_scalar((type == NetNet::IMPLICIT) ? SR_PORT : SR_BOTH);

      } else {
	    cur->set_range(*prange, (type == NetNet::IMPLICIT) ? SR_PORT : SR_BOTH);
      }

      if (urange) {
	    cur->set_unpacked_idx(*urange);
      }

      pform_bind_attributes(cur->attributes, attr, keep_attr);
      pform_put_wire_in_scope(name, cur);
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
	    if (cur->udims)
		  use_type = new uarray_type_t(vtype, cur->udims);

	    pform_module_define_port(li, cur->name, port_kind, type, use_type,
				     attr, true);
	    if (cur->udims)
		  delete use_type;

	    if (cur->expr)
		  pform_make_var_init(li, cur->name, cur->expr);
      }

      delete ports;
      delete attr;
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

	// If the wire already exists but isn't yet fully defined,
	// carry on adding details.
      if (cur && (cur->get_data_type() == IVL_VT_NO_TYPE ||
                  cur->get_wire_type() == NetNet::IMPLICIT) ) {
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

	// If the wire already exists and is fully defined, this
	// must be a redeclaration. Start again with a new wire.
	// The error will be reported when we add the new wire
	// to the scope. Do not delete the old wire - it will
	// remain in the local symbol map.

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
		    list<pform_range_t>*range,
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
		  pform_set_net_range(txt, type, range, signed_flag, dt, rt, 0);
	    }
      }

      delete names;
      delete range;
      delete attr;
}

/*
 * This form makes nets with delays and continuous assignments.
 */
void pform_makewire(const vlltype&li,
		    list<PExpr*>*delay,
		    str_pair_t str,
		    net_decl_assign_t*decls,
		    NetNet::Type type,
		    data_type_t*data_type)
{
	// The decls pointer is a circularly linked list.
      net_decl_assign_t*first = decls->next;

      list<perm_string>*names = new list<perm_string>;

	// Go through the circularly linked list non-destructively.
      do {
	    pform_makewire(li, first->name, type, NetNet::NOT_A_PORT, IVL_VT_NO_TYPE, 0);
	    names->push_back(first->name);
	    first = first->next;
      } while (first != decls->next);

	// The pform_set_data_type function will delete the names list.
      pform_set_data_type(li, data_type, names, type, 0);

	// This time, go through the list, deleting cells as I'm done.
      first = decls->next;
      decls->next = 0;
      while (first) {
	    net_decl_assign_t*next = first->next;
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
 * This should eventually replace the form above that takes a
 * net_decl_assign_t argument.
 */
void pform_makewire(const struct vlltype&li,
		    std::list<PExpr*>*delay,
		    str_pair_t str,
		    std::list<decl_assignment_t*>*assign_list,
		    NetNet::Type type,
		    data_type_t*data_type)
{
      if (is_compilation_unit(lexical_scope) && !gn_system_verilog()) {
	    VLerror(li, "error: variable declarations must be contained within a module.");
	    return;
      }

      list<perm_string>*names = new list<perm_string>;

      for (list<decl_assignment_t*>::iterator cur = assign_list->begin()
		 ; cur != assign_list->end() ; ++ cur) {
	    decl_assignment_t* curp = *cur;
	    pform_makewire(li, curp->name, type, NetNet::NOT_A_PORT, IVL_VT_NO_TYPE, 0);
	    pform_set_reg_idx(curp->name, &curp->index);
	    names->push_back(curp->name);
      }

      pform_set_data_type(li, data_type, names, type, 0);

      while (! assign_list->empty()) {
	    decl_assignment_t*first = assign_list->front();
	    assign_list->pop_front();
            if (PExpr*expr = first->expr.release()) {
                  if (type == NetNet::REG || type == NetNet::IMPLICIT_REG) {
                        pform_make_var_init(li, first->name, expr);
                  } else {
		        PEIdent*lval = new PEIdent(first->name);
		        FILE_NAME(lval, li.text, li.first_line);
		        PGAssign*ass = pform_make_pgassign(lval, expr, delay, str);
		        FILE_NAME(ass, li.text, li.first_line);
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
				     ivl_variable_type_t vtype,
				     bool signed_flag,
				     list<pform_range_t>*range,
				     list<perm_string>*names,
				     bool isint)
{
      assert(pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);
      assert(names);
      vector<pform_tf_port_t>*res = new vector<pform_tf_port_t>(0);
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
		if (isint) {
			bool flag = curw->set_wire_type(NetNet::INTEGER);
			assert(flag);
		}

	      /* If there is a range involved, it needs to be set. */
	    if (range) {
		  curw->set_range(*range, SR_PORT);
	    }

	    res->push_back(pform_tf_port_t(curw));
      }

      delete range;
      return res;
}

static vector<pform_tf_port_t>*do_make_task_ports(const struct vlltype&loc,
					 NetNet::PortType pt,
					 ivl_variable_type_t var_type,
					 data_type_t*data_type,
					 list<perm_string>*names)
{
      assert(pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);
      assert(names);
      vector<pform_tf_port_t>*res = new vector<pform_tf_port_t>(0);

      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++cur) {
	    perm_string name = *cur;
	    PWire*curw = pform_get_wire_in_scope(name);
	    if (curw) {
		  curw->set_port_type(pt);
	    } else {
		  curw = new PWire(name, NetNet::IMPLICIT_REG, pt, var_type);
		  FILE_NAME(curw, loc);
		  curw->set_data_type(data_type);
		  pform_put_wire_in_scope(name, curw);
	    }

	    res->push_back(pform_tf_port_t(curw));
      }
      return res;
}

vector<pform_tf_port_t>*pform_make_task_ports(const struct vlltype&loc,
				      NetNet::PortType pt,
				      data_type_t*vtype,
				      list<perm_string>*names)
{
      vector<pform_tf_port_t>*ret = NULL;
      std::list<pform_range_t>*unpacked_dims = NULL;

      if (uarray_type_t*uarray = dynamic_cast<uarray_type_t*> (vtype)) {
            unpacked_dims = uarray->dims.get();
            vtype = uarray->base_type;
      }

      if (atom2_type_t*atype = dynamic_cast<atom2_type_t*> (vtype)) {
	    list<pform_range_t>*range_tmp = make_range_from_width(atype->type_code);
	    ret = pform_make_task_ports(loc, pt, IVL_VT_BOOL,
					 atype->signed_flag,
					 range_tmp, names);
      }

      if (vector_type_t*vec_type = dynamic_cast<vector_type_t*> (vtype)) {
	    ret = pform_make_task_ports(loc, pt, vec_type->base_type,
					 vec_type->signed_flag,
					 copy_range(vec_type->pdims.get()),
					 names, vec_type->integer_flag);
      }

      if (/*real_type_t*real_type = */ dynamic_cast<real_type_t*> (vtype)) {
	    ret = pform_make_task_ports(loc, pt, IVL_VT_REAL,
					 true, 0, names);
      }

      if (dynamic_cast<string_type_t*> (vtype)) {
	    ret = pform_make_task_ports(loc, pt, IVL_VT_STRING,
					 false, 0, names);
      }

      if (class_type_t*class_type = dynamic_cast<class_type_t*> (vtype)) {
	    ret = do_make_task_ports(loc, pt, IVL_VT_CLASS, class_type, names);
      }

      if (! ret) {
	    ret = do_make_task_ports(loc, pt, IVL_VT_NO_TYPE, vtype, names);
      }

      if (unpacked_dims) {
	    for (list<perm_string>::iterator cur = names->begin()
                    ; cur != names->end() ; ++ cur ) {
		PWire*wire = pform_get_wire_in_scope(*cur);
		wire->set_unpacked_idx(*unpacked_dims);
	    }
      }

      delete names;
      return ret;
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
      if (!gn_system_verilog()) {
	    cerr << loc << ": error: Increment/decrement operators "
		    "require SystemVerilog." << endl;
	    error_count += 1;
      }

      PExpr*lval = new PEIdent(lex_strings.make(name));
      PExpr*rval = new PENumber(new verinum((uint64_t)1, 1));
      FILE_NAME(lval, loc);
      FILE_NAME(rval, loc);

      PEBinary*tmp = new PEBinary(inc_flag ? '+' : '-', lval, rval);
      FILE_NAME(tmp, loc);

      return tmp;
}

void pform_set_attrib(perm_string name, perm_string key, char*value)
{
      if (PWire*cur = lexical_scope->wires_find(name)) {
	    cur->attributes[key] = new PEString(value);

      } else if (PGate*curg = pform_cur_module.front()->get_gate(name)) {
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
void pform_set_reg_idx(perm_string name, list<pform_range_t>*indices)
{
      PWire*cur = lexical_scope->wires_find(name);
      if (cur == 0) {
	    VLerror("internal error: name is not a valid memory for index.");
	    return;
      }

      if (indices && !indices->empty())
	    cur->set_unpacked_idx(*indices);
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
			 bool signed_flag, list<pform_range_t>*range, PExpr*expr,
			 LexicalScope::range_t*value_range)
{
      LexicalScope*scope = lexical_scope;
      if (is_compilation_unit(scope) && !gn_system_verilog()) {
	    VLerror(loc, "error: parameter declarations must be contained within a module.");
	    return;
      }
      if (scope == pform_cur_generate) {
            VLerror("parameter declarations are not permitted in generate blocks");
            return;
      }

      assert(expr);
      Module::param_expr_t*parm = new Module::param_expr_t();
      FILE_NAME(parm, loc);

      add_local_symbol(scope, name, parm);
      scope->parameters[name] = parm;

      parm->expr = expr;

      parm->type = type;
      if (range) {
	    assert(range->size() == 1);
	    pform_range_t&rng = range->front();
	    assert(rng.first);
	    assert(rng.second);
	    parm->msb = rng.first;
	    parm->lsb = rng.second;
      } else {
	    parm->msb = 0;
	    parm->lsb = 0;
      }
      parm->signed_flag = signed_flag;
      parm->range = value_range;

	// Only a Module keeps the position of the parameter.
      if ((dynamic_cast<Module*>(scope)) && (scope == pform_cur_module.front()))
            pform_cur_module.front()->param_names.push_back(name);
}

void pform_set_localparam(const struct vlltype&loc,
			  perm_string name, ivl_variable_type_t type,
			  bool signed_flag, list<pform_range_t>*range, PExpr*expr)
{
      LexicalScope*scope = lexical_scope;
      if (is_compilation_unit(scope) && !gn_system_verilog()) {
	    VLerror(loc, "error: localparam declarations must be contained within a module.");
	    return;
      }

      assert(expr);
      Module::param_expr_t*parm = new Module::param_expr_t();
      FILE_NAME(parm, loc);

      add_local_symbol(scope, name, parm);
      scope->localparams[name] = parm;

      parm->expr = expr;

      parm->type = type;
      if (range) {
	    assert(range->size() == 1);
	    pform_range_t&rng = range->front();
	    assert(rng.first);
	    assert(rng.second);
	    parm->msb = rng.first;
	    parm->lsb = rng.second;
      } else {
	    parm->msb  = 0;
	    parm->lsb  = 0;
      }
      parm->signed_flag = signed_flag;
      parm->range = 0;
}

void pform_set_specparam(const struct vlltype&loc, perm_string name,
			 list<pform_range_t>*range, PExpr*expr)
{
      assert(! pform_cur_module.empty());
      Module*scope = pform_cur_module.front();
      assert(scope == lexical_scope);

      assert(expr);
      Module::param_expr_t*parm = new Module::param_expr_t();
      FILE_NAME(parm, loc);

      add_local_symbol(scope, name, parm);
      pform_cur_module.front()->specparams[name] = parm;

      parm->expr = expr;

      if (range) {
	    assert(range->size() == 1);
	    pform_range_t&rng = range->front();
	    assert(rng.first);
	    assert(rng.second);
	    parm->type = IVL_VT_LOGIC;
	    parm->msb = rng.first;
	    parm->lsb = rng.second;
      } else {
	    parm->type = IVL_VT_NO_TYPE;
	    parm->msb  = 0;
	    parm->lsb  = 0;
      }
      parm->signed_flag = false;
      parm->range = 0;
}

void pform_set_defparam(const pform_name_t&name, PExpr*expr)
{
      assert(expr);
      if (pform_cur_generate)
            pform_cur_generate->defparms.push_back(make_pair(name,expr));
      else
            pform_cur_module.front()->defparms.push_back(make_pair(name,expr));
}

void pform_set_param_from_type(const struct vlltype&loc,
                               const data_type_t *data_type,
                               const char *name,
                               list<pform_range_t> *&param_range,
                               bool &param_signed,
                               ivl_variable_type_t &param_type)
{
      if (const vector_type_t *vec = dynamic_cast<const vector_type_t*> (data_type)) {
	    param_range = vec->pdims.get();
	    param_signed = vec->signed_flag;
	    param_type = vec->base_type;
	    return;
      }

      param_range = 0;
      param_signed = false;
      param_type = IVL_VT_NO_TYPE;
      cerr << loc.get_fileline() << ": sorry: cannot currently create a "
              "parameter of type '" << name << "' which was defined at: "
           << data_type->get_fileline() <<  "." << endl;
      error_count += 1;
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
      for (idx = 0, cur = src->begin() ;  cur != src->end() ;  ++ idx, ++ cur) {
	    path->src[idx] = *cur;
      }
      assert(idx == path->src.size());
      delete src;

      for (idx = 0, cur = dst->begin() ;  cur != dst->end() ;  ++ idx, ++ cur) {
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
      pform_cur_module.front()->specify_paths.push_back(obj);
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
			 list<pform_port_t>*ports,
			 NetNet::PortType pt,
			 data_type_t*dt,
			 list<named_pexpr_t>*attr)
{
      assert(pt != NetNet::PIMPLICIT && pt != NetNet::NOT_A_PORT);

      list<pform_range_t>*range = 0;
      bool signed_flag = false;
      if (vector_type_t*vt = dynamic_cast<vector_type_t*> (dt)) {
	    assert(vt->implicit_flag);
	    range = vt->pdims.get();
	    signed_flag = vt->signed_flag;
      } else {
	    assert(dt == 0);
      }

      bool have_init_expr = false;
      for (list<pform_port_t>::iterator cur = ports->begin()
		 ; cur != ports->end() ; ++ cur ) {

	    pform_set_port_type(cur->name, pt, li.text, li.first_line);
	    pform_set_net_range(cur->name, NetNet::NONE, range, signed_flag,
				IVL_VT_NO_TYPE, SR_PORT, attr);
	    if (cur->udims) {
		  cerr << li.text << ":" << li.first_line << ": warning: "
		       << "Array dimensions in incomplete port declarations "
		       << "are currently ignored." << endl;
		  cerr << li.text << ":" << li.first_line << ":        : "
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
	    cerr << li.text << ":" << li.first_line << ": error: "
		 << "Incomplete port declarations cannot be initialized."
		 << endl;
	    error_count += 1;
      }

      delete ports;
      delete dt;
      delete attr;
}

static void pform_set_integer_2atom(const struct vlltype&li, uint64_t width, bool signed_flag, perm_string name, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      PWire*cur = pform_get_make_wire_in_scope(li, name, net_type, NetNet::NOT_A_PORT, IVL_VT_BOOL);
      assert(cur);

      cur->set_signed(signed_flag);

      pform_range_t rng;
      rng.first = new PENumber(new verinum(width-1, integer_width));
      rng.second = new PENumber(new verinum((uint64_t)0, integer_width));
      list<pform_range_t>rlist;
      rlist.push_back(rng);
      cur->set_range(rlist, SR_NET);
      pform_bind_attributes(cur->attributes, attr, true);
}

static void pform_set_integer_2atom(const struct vlltype&li, uint64_t width, bool signed_flag, list<perm_string>*names, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_set_integer_2atom(li, width, signed_flag, txt, net_type, attr);
      }
}

template <class T> static void pform_set2_data_type(const struct vlltype&li, T*data_type, perm_string name, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      ivl_variable_type_t base_type = data_type->figure_packed_base_type();
      if (base_type == IVL_VT_NO_TYPE) {
	    VLerror(li, "Compound type is not PACKED in this context.");
      }

      PWire*net = pform_get_make_wire_in_scope(li, name, net_type, NetNet::NOT_A_PORT, base_type);
      assert(net);
      pform_bind_attributes(net->attributes, attr, true);
}

template <class T> static void pform_set2_data_type(const struct vlltype&li, T*data_type, list<perm_string>*names, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur) {
	    pform_set2_data_type(li, data_type, *cur, net_type, attr);
      }
}

static void pform_set_enum(const struct vlltype&li, enum_type_t*enum_type,
			   perm_string name, NetNet::Type net_type,
			   std::list<named_pexpr_t>*attr)
{
      PWire*cur = pform_get_make_wire_in_scope(li, name, net_type, NetNet::NOT_A_PORT, enum_type->base_type);
      assert(cur);

      cur->set_signed(enum_type->signed_flag);

      assert(enum_type->range.get() != 0);
      assert(enum_type->range->size() == 1);
	//XXXXcur->set_range(*enum_type->range, SR_NET);
	// If this is an integer enumeration switch the wire to an integer.
      if (enum_type->integer_flag) {
	    bool res = cur->set_wire_type(NetNet::INTEGER);
	    assert(res);
      }
      pform_bind_attributes(cur->attributes, attr, true);
}

static void pform_set_enum(const struct vlltype&li, enum_type_t*enum_type,
			   list<perm_string>*names, NetNet::Type net_type,
			   std::list<named_pexpr_t>*attr)
{
	// By definition, the base type can only be IVL_VT_LOGIC or
	// IVL_VT_BOOL.
      assert(enum_type->base_type==IVL_VT_LOGIC || enum_type->base_type==IVL_VT_BOOL);

      assert(enum_type->range.get() != 0);
      assert(enum_type->range->size() == 1);

	// Add the file and line information to the enumeration type.
      FILE_NAME(&(enum_type->li), li);

	// If this is an anonymous enumeration, attach it to the current scope.
      if (enum_type->name.nil())
	    pform_put_enum_type_in_scope(enum_type);

	// Now apply the checked enumeration type to the variables
	// that are being declared with this type.
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur) {
	    perm_string txt = *cur;
	    pform_set_enum(li, enum_type, txt, net_type, attr);
      }

}

/*
 * This function detects the derived class for the given type and
 * dispatches the type to the proper subtype function.
 */
void pform_set_data_type(const struct vlltype&li, data_type_t*data_type, list<perm_string>*names, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      if (data_type == 0) {
	    VLerror(li, "internal error: data_type==0.");
	    assert(0);
      }

      uarray_type_t*uarray_type = dynamic_cast<uarray_type_t*> (data_type);
      if (uarray_type)
            data_type = uarray_type->base_type;

      if (atom2_type_t*atom2_type = dynamic_cast<atom2_type_t*> (data_type)) {
	    pform_set_integer_2atom(li, atom2_type->type_code, atom2_type->signed_flag, names, net_type, attr);
      }

      else if (struct_type_t*struct_type = dynamic_cast<struct_type_t*> (data_type)) {
	    pform_set_struct_type(li, struct_type, names, net_type, attr);
      }

      else if (enum_type_t*enum_type = dynamic_cast<enum_type_t*> (data_type)) {
	    pform_set_enum(li, enum_type, names, net_type, attr);
      }

      else if (vector_type_t*vec_type = dynamic_cast<vector_type_t*> (data_type)) {
	    if (net_type==NetNet::REG && vec_type->integer_flag)
		  net_type=NetNet::INTEGER;

	    pform_set_net_range(names, vec_type->pdims.get(),
				vec_type->signed_flag,
				vec_type->base_type, net_type, attr);
      }

      else if (/*real_type_t*real_type =*/ dynamic_cast<real_type_t*> (data_type)) {
	    pform_set_net_range(names, 0, true, IVL_VT_REAL, net_type, attr);
      }

      else if (class_type_t*class_type = dynamic_cast<class_type_t*> (data_type)) {
	    pform_set_class_type(li, class_type, names, net_type, attr);
      }

      else if (parray_type_t*array_type = dynamic_cast<parray_type_t*> (data_type)) {
	    pform_set2_data_type(li, array_type, names, net_type, attr);
      }

      else if (string_type_t*string_type = dynamic_cast<string_type_t*> (data_type)) {
	    pform_set_string_type(li, string_type, names, net_type, attr);

      } else {
	    VLerror(li, "internal error: Unexpected data_type.");
	    assert(0);
      }

      for (list<perm_string>::iterator cur = names->begin()
	      ; cur != names->end() ; ++ cur ) {
	    PWire*wire = pform_get_wire_in_scope(*cur);
	    if (uarray_type) {
		  wire->set_unpacked_idx(*uarray_type->dims.get());
		  wire->set_uarray_type(uarray_type);
	    }
	    wire->set_data_type(data_type);
      }

      delete names;
}

vector<PWire*>* pform_make_udp_input_ports(list<perm_string>*names)
{
      vector<PWire*>*out = new vector<PWire*>(names->size());

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
	// Add an implicit @* around the statement for the always_comb and
	// always_latch statements.
      if ((type == IVL_PR_ALWAYS_COMB) || (type == IVL_PR_ALWAYS_LATCH)) {
	    PEventStatement *tmp = new PEventStatement(true);
	    tmp->set_file(st->get_file());
	    tmp->set_lineno(st->get_lineno());
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
		  sprintf(unit_name, "$unit#%u", ++nunits);
	    else
		  sprintf(unit_name, "$unit");

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
