/*
 * Copyright (c) 2012-2024 Stephen Williams (steve@icarus.com)
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
# include  "pform.h"
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
	    data_type_t*use_type = data_type;

	    if (! curp->index.empty()) {
		  list<pform_range_t>*pd = new list<pform_range_t> (curp->index);
		  use_type = new uarray_type_t(use_type, pd);
		  FILE_NAME(use_type, loc);
	    }

	    pform_cur_class->type->properties[curp->name.first]
		  = class_type_t::prop_info_t(property_qual,use_type);
	    FILE_NAME(&pform_cur_class->type->properties[curp->name.first], loc);

	    if (PExpr*rval = curp->expr.release()) {
		  PExpr*lval = new PEIdent(curp->name.first, curp->name.second);
		  FILE_NAME(lval, loc);
		  PAssign*tmp = new PAssign(lval, rval);
		  FILE_NAME(tmp, loc);

		  if (property_qual.test_static())
			pform_cur_class->type->initialize_static.push_back(tmp);
		  else
			pform_cur_class->type->initialize.push_back(tmp);
	    }
      }
}

void pform_set_this_class(const struct vlltype&loc, PTaskFunc*net)
{
      if (pform_cur_class == 0)
	    return;

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
