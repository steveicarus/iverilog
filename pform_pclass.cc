/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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

# include  "pform.h"
# include  "PClass.h"
# include  "parse_misc.h"

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
void pform_start_class_declaration(const struct vlltype&loc, class_type_t*type, data_type_t*base_type, list<PExpr*>*base_exprs)
{
      PClass*class_scope = pform_push_class_scope(loc, type->name);
      class_scope->type = type;
      assert(pform_cur_class == 0);
      pform_cur_class = class_scope;

      assert(type->base_type == 0);
      type->base_type = base_type;

      assert(type->base_args.empty());
      if (base_exprs) {
	    for (list<PExpr*>::iterator cur = base_exprs->begin()
		       ; cur != base_exprs->end() ; ++ cur) {
		  type->base_args.push_back(*cur);
	    }
	    delete base_exprs;
      }
}

void pform_class_property(const struct vlltype&loc,
			  property_qualifier_t property_qual,
			  data_type_t*data_type,
			  list<decl_assignment_t*>*decls)
{
      assert(pform_cur_class);

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
	    }

	    pform_cur_class->type->properties[curp->name]
		  = class_type_t::prop_info_t(property_qual,use_type);

	    if (PExpr*rval = curp->expr.release()) {
		  PExpr*lval = new PEIdent(curp->name);
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

      list<perm_string>*this_name = new list<perm_string>;
      this_name->push_back(perm_string::literal("@"));
      vector<pform_tf_port_t>*this_port = pform_make_task_ports(loc,
						       NetNet::PINPUT,
						       pform_cur_class->type,
						       this_name);
	// The pform_make_task_ports() function deletes the this_name
	// object.

      assert(this_port->at(0).defe == 0);
      PWire*this_wire = this_port->at(0).port;
      delete this_port;

      net->set_this(pform_cur_class->type, this_wire);
}

void pform_set_constructor_return(PFunction*net)
{
      assert(pform_cur_class);
      net->set_return(pform_cur_class->type);
}

/*
 * A constructor is basically a function with special implications.
 */
PFunction*pform_push_constructor_scope(const struct vlltype&loc)
{
      assert(pform_cur_class);
      PFunction*func = pform_push_function_scope(loc, "new", true);
      return func;
}

void pform_end_class_declaration(const struct vlltype&loc)
{
      assert(pform_cur_class);

	// If there were initializer statements, then collect them
	// into an implicit constructor function.
      if (! pform_cur_class->type->initialize.empty()) {
	    PFunction*func = pform_push_function_scope(loc, "new@", true);
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
