/*
 * Copyright (c) 2012-2026 Stephen Williams (steve@icarus.com)
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

# include  <cstdarg>
# include  <utility>
# include  "pform.h"
# include  "PGenerate.h"
# include  "PClass.h"
# include  "parse_misc.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * The functions here help the parser put together class type declarations.
 */
static PClass*pform_cur_class = 0;

/*
 * The base_type is set to the base class if this declaration is
 * starting a derived class. For example, for the syntax:
 *
 *    class foo extends bar (exprs) ...
 *
 * the base_type is the type of the class "bar", and the base_exprs,
 * if present, are the "exprs" that would be passed to a chained
 * constructor.
 */
void pform_start_class_declaration(const struct vlltype&loc,
				   class_type_t*type,
				   data_type_t*base_type,
				   list<named_pexpr_t> *base_args,
				   bool virtual_class)
{
      PClass*class_scope = pform_push_class_scope(loc, type->name);
      class_scope->type = type;
      ivl_assert(loc, pform_cur_class == 0);
      pform_cur_class = class_scope;

      ivl_assert(loc, type->base_type == 0);
      type->base_type.reset(base_type);
      type->virtual_class = virtual_class;


      ivl_assert(loc, type->base_args.empty());
      if (base_args) {
	    type->base_args.insert(type->base_args.begin(), base_args->begin(),
			           base_args->end());
	    delete base_args;
      }
}

void pform_class_property(const struct vlltype&loc,
			  property_qualifier_t property_qual,
			  data_type_t*data_type,
			  list<decl_assignment_t*>*decls)
{
      ivl_assert(loc, pform_cur_class);

	// Add the non-static properties to the class type
	// object. Unwind the list of names to make a map of name to
	// type.
      for (list<decl_assignment_t*>::iterator cur = decls->begin()
		 ; cur != decls->end() ; ++cur) {

	    decl_assignment_t*curp = *cur;
	    class_type_t::prop_info_t property(property_qual, nullptr,
						 curp->expr != nullptr);
	    property.set_line(*curp);

	    // Properties are stored by value in the property map, while the type
	    // can be shared by comma-separated declarations. Check the temporary
	    // before taking ownership of the type or releasing the initializer. It
	    // can not be registered in the local symbol table yet since that would
	    // store a pointer to the temporary.
	    if (!pform_check_local_symbol(pform_cur_class, curp->name.first,
					  &property))
		  continue;

	    auto result = pform_cur_class->type->properties.emplace(
		  curp->name.first, std::move(property));
	    ivl_assert(loc, result.second);

	    auto &property_info = result.first->second;
	    // Register the property now that it has a stable address. The earlier
	    // check makes failure of this insertion an internal error.
	    auto symbol = pform_cur_class->local_symbols.emplace(
		  curp->name.first, &property_info);
	    ivl_assert(loc, symbol.second);

	    data_type_t*use_type = data_type;

	    if (! curp->index.empty()) {
		  list<pform_range_t>*pd = new list<pform_range_t> (curp->index);
		  use_type = new uarray_type_t(use_type, pd);
		  FILE_NAME(use_type, loc);
	    }

	    property_info.type.reset(use_type);

	    if (PExpr*rval = curp->expr.release()) {
		  PExpr*lval = new PEIdent(curp->name.first, curp->name.second);
		  FILE_NAME(lval, loc);
		  auto tmp = new PAssign(lval, rval, false, true);
		  FILE_NAME(tmp, loc);

		  if (property_qual.test_static())
			pform_cur_class->type->initialize_static.push_back(tmp);
		  else
			pform_cur_class->type->initialize.push_back(tmp);
	    }
      }
}

/*
 * The "virtual" method qualifier is parsed before the function or task
 * scope exists, so remember it here until pform_set_this_class() sees
 * the method.
 */
static bool method_virtual_flag = false;

void pform_set_method_virtual(bool flag)
{
      method_virtual_flag = flag;
}

void pform_set_this_class(const struct vlltype&loc, PTaskFunc*net)
{
      if (pform_cur_class == 0)
	    return;

      net->set_virtual(method_virtual_flag);
      method_virtual_flag = false;

      list<pform_port_t>*this_name = new list<pform_port_t>;
      this_name->push_back(pform_port_t({ perm_string::literal(THIS_TOKEN), 0 }, 0, 0));
      vector<pform_tf_port_t>*this_port = pform_make_task_ports(loc,
						       NetNet::PINPUT,
						       pform_cur_class->type,
						       this_name);
	// The pform_make_task_ports() function deletes the this_name
	// object.

      ivl_assert(loc, this_port->at(0).defe == 0);
      PWire*this_wire = this_port->at(0).port;
      delete this_port;

      net->set_this(pform_cur_class->type, this_wire);
}

/*
 * An "extern" method prototype in a class body promises a definition
 * outside the class: "function type class::name(...)".
 */
void pform_class_extern_method(const struct vlltype&loc, const char*name,
			       bool is_task, bool is_virtual,
			       vector<pform_tf_port_t>*ports,
			       data_type_t*ret_type)
{
      ivl_assert(loc, pform_cur_class);
      method_virtual_flag = false;

      PClass::extern_method_t info;
      FILE_NAME(&info, loc);
      info.is_task = is_task;
      info.is_virtual = is_virtual;
      info.defined = false;
      info.ports = ports;
      info.ret_type = ret_type;

      perm_string use_name = lex_strings.make(name);
      if (!pform_cur_class->externs.emplace(use_name, info).second) {
	    cerr << loc << ": error: Extern method '" << use_name
		 << "' is already declared in class '"
		 << pform_cur_class->pscope_name() << "'." << endl;
	    error_count += 1;
      }
}

/*
 * While the body of an out-of-block method definition is parsed, the
 * lexical scope is the class scope, so the method is stored exactly
 * like a method written inside the class. The scope is restored when
 * the method scope is popped.
 */
static PClass*extern_class = 0;
static LexicalScope*extern_saved_scope = 0;
static PClass*extern_saved_class = 0;
static PClass::extern_method_t*extern_proto = 0;

/*
 * On an error, parse the method into a throw-away class so that it does
 * not end up in the enclosing scope and cause follow-on errors.
 */
static bool extern_method_error(LexicalScope*scope)
{
      extern_class = new PClass(lex_strings.make("$extern_error"), scope);
      extern_saved_scope = pform_set_scope(extern_class);
      extern_saved_class = pform_cur_class;
      extern_proto = 0;
      return false;
}

bool pform_start_extern_method(const struct vlltype&loc, const char*class_name,
			       const char*name, bool is_task)
{
      perm_string use_class = lex_strings.make(class_name);
      perm_string use_name = lex_strings.make(name);

      if (pform_cur_class) {
	    cerr << loc << ": error: Out-of-block definition of '"
		 << use_class << "::" << use_name
		 << "' is not allowed inside a class." << endl;
	    error_count += 1;
	    return extern_method_error(pform_peek_scope());
      }

	// The definition must be in the scope that declares the class.
      LexicalScope*scope = pform_peek_scope();
      PClass*pclass = 0;
      if (auto scopex = dynamic_cast<PScopeExtra*> (scope)) {
	    auto found = scopex->classes.find(use_class);
	    if (found != scopex->classes.end())
		  pclass = found->second;
      } else if (auto gen = dynamic_cast<PGenerate*> (scope)) {
	    auto found = gen->classes.find(use_class);
	    if (found != gen->classes.end())
		  pclass = found->second;
      }

      if (!pclass) {
	    cerr << loc << ": error: Unknown class '" << use_class
		 << "' in definition of '" << use_class << "::" << use_name
		 << "'. The class must be declared in the same scope."
		 << endl;
	    error_count += 1;
	    return extern_method_error(scope);
      }

      auto proto = pclass->externs.find(use_name);
      if (proto == pclass->externs.end()) {
	    cerr << loc << ": error: Class '" << use_class
		 << "' has no extern method '" << use_name
		 << "' to define." << endl;
	    error_count += 1;
	    return extern_method_error(pform_peek_scope());
      }

      if (proto->second.is_task != is_task) {
	    cerr << loc << ": error: '" << use_class << "::" << use_name
		 << "' is defined as a " << (is_task? "task" : "function")
		 << " but was declared extern as a "
		 << (proto->second.is_task? "task" : "function") << " at "
		 << proto->second.get_fileline() << "." << endl;
	    error_count += 1;
	    return extern_method_error(pform_peek_scope());
      }

      if (proto->second.defined) {
	    cerr << loc << ": error: Extern method '" << use_class << "::"
		 << use_name << "' is already defined at "
		 << proto->second.def.get_fileline() << "." << endl;
	    error_count += 1;
	    return extern_method_error(pform_peek_scope());
      }
      proto->second.defined = true;
      FILE_NAME(&proto->second.def, loc);
      extern_proto = &proto->second;

      extern_class = pclass;
      extern_saved_scope = pform_set_scope(pclass);
      extern_saved_class = pform_cur_class;
      pform_cur_class = pclass;
      method_virtual_flag = proto->second.is_virtual;
      return true;
}

/*
 * Check the ports of an out-of-block definition against the extern
 * prototype. A default value that only the prototype gives is taken
 * over by the definition.
 */
void pform_check_extern_ports(const struct vlltype&loc,
			      vector<pform_tf_port_t>*ports)
{
      if (extern_proto == 0)
	    return;

      static const vector<pform_tf_port_t> none;
      const vector<pform_tf_port_t>&pp = extern_proto->ports? *extern_proto->ports : none;
      const vector<pform_tf_port_t>&dp = ports? *ports : none;
      const char*what = extern_proto->is_task? "task" : "function";
      const string where = extern_proto->get_fileline();
      FILE_NAME(&extern_proto->def, loc);

      if (pp.size() != dp.size()) {
	    cerr << loc << ": error: Definition of extern " << what
		 << " has " << dp.size() << " port(s) but the prototype at "
		 << where << " has " << pp.size() << "." << endl;
	    error_count += 1;
	    return;
      }

      for (size_t idx = 0 ; idx < pp.size() ; idx += 1) {
	    PWire*pw = pp[idx].port;
	    PWire*dw = dp[idx].port;
	    if (pw->basename() != dw->basename()) {
		  cerr << loc << ": error: Port " << idx+1 << " of extern "
		       << what << " is named '" << dw->basename()
		       << "' but the prototype at " << where << " names it '"
		       << pw->basename() << "'." << endl;
		  error_count += 1;
	    } else if (pw->get_port_type() != dw->get_port_type()) {
		  cerr << loc << ": error: Port '" << dw->basename()
		       << "' of extern " << what << " has a different "
		       << "direction than in the prototype at " << where
		       << "." << endl;
		  error_count += 1;
	    }
	    if (!dp[idx].defe)
		  (*ports)[idx].defe = pp[idx].defe;
      }
}

PFunction*pform_push_extern_constructor_scope(const struct vlltype&loc,
					      const char*class_name)
{
      if (pform_start_extern_method(loc, class_name, "new", false))
	    return pform_push_constructor_scope(loc);

      return pform_push_function_scope(loc, "new", LexicalScope::AUTOMATIC);
}

void pform_end_extern_method()
{
      if (extern_class == 0 || pform_peek_scope() != extern_class)
	    return;
      extern_proto = 0;

      pform_set_scope(extern_saved_scope);
      extern_class = 0;
      extern_saved_scope = 0;
      pform_cur_class = extern_saved_class;
      extern_saved_class = 0;
}

void pform_set_constructor_return(PFunction*net)
{
      ivl_assert(*net, pform_cur_class);
      net->set_return(pform_cur_class->type);
}

/*
 * A constructor is basically a function with special implications.
 */
PFunction*pform_push_constructor_scope(const struct vlltype&loc)
{
      ivl_assert(loc, pform_cur_class);
      PFunction*func = pform_push_function_scope(loc, "new", LexicalScope::AUTOMATIC);
      return func;
}

void pform_end_class_declaration(const struct vlltype&loc)
{
      ivl_assert(loc, pform_cur_class);

	// If there were initializer statements, then collect them
	// into an implicit constructor function. Also make sure that an
	// explicit constructor exists if base class constructor arguments are
	// specified, so that the base class constructor will be called.
      if (!pform_cur_class->type->initialize.empty() ||
          !pform_cur_class->type->base_args.empty()) {
	    PFunction*func = pform_push_function_scope(loc, "new@", LexicalScope::AUTOMATIC);
	    func->set_ports(0);
	    pform_set_constructor_return(func);
	    pform_set_this_class(loc, func);

	    class_type_t*use_class = pform_cur_class->type;
	    if (use_class->initialize.size() == 1) {
		  func->set_statement(use_class->initialize.front());
	    } else {
		  PBlock*tmp = new PBlock(PBlock::BL_SEQ);
		  tmp->set_statement(use_class->initialize);
		  func->set_statement(tmp);
	    }
	    pform_pop_scope();
      }

      pform_cur_class = 0;
      pform_pop_scope();
}

bool pform_in_class()
{
      return pform_cur_class != 0;
}
