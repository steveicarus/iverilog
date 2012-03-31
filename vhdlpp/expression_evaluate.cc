/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "expression.h"
# include  "architec.h"
# include  <ivl_assert.h>

bool Expression::evaluate(ScopeBase*, int64_t&) const
{
      return false;
}

bool Expression::evaluate(Entity*, Architecture*arc, int64_t&val) const
{
      return evaluate(arc, val);
}


bool ExpAttribute::evaluate(ScopeBase*, int64_t&val) const
{
	/* Special Case: The length attribute can be calculated all
	   the down to a literal integer at compile time, and all it
	   needs is the type of the base expression. (The base
	   expression doesn't even need to be evaluated.) */
      if (name_ == "length") {
	    const VType*base_type = base_->peek_type();
	      //if (base_type == 0)
	      //	  base_type = base_->probe_type(ent,arc);

	    ivl_assert(*this, base_type);

	    const VTypeArray*arr = dynamic_cast<const VTypeArray*>(base_type);
	    if (arr == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Cannot apply the 'length attribute to non-array objects"
		       << endl;
		  return false;
	    }

	    int64_t size = 1;
	    for (size_t idx = 0 ; idx < arr->dimensions() ; idx += 1) {
		  const VTypeArray::range_t&dim = arr->dimension(idx);
		  ivl_assert(*this, ! dim.is_box());
		  size *= 1 + labs(dim.msb() - dim.lsb());
	    }
	    val = size;
	    return true;
      }

      return false;
}

bool ExpAttribute::evaluate(Entity*, Architecture*arc, int64_t&val) const
{
      return evaluate(arc, val);
}

bool ExpName::evaluate(ScopeBase*scope, int64_t&val) const
{
      const VType*type;
      Expression*exp;

      if (prefix_.get()) {
	    cerr << get_fileline() << ": sorry: I don't know how to evaluate ExpName prefix parts." << endl;
	    return false;
      }

      bool rc = scope->find_constant(name_, type, exp);
      if (rc == false)
	    return false;

      return exp->evaluate(scope, val);
}

bool ExpName::evaluate(Entity*ent, Architecture*arc, int64_t&val) const
{
      if (prefix_.get()) {
	    cerr << get_fileline() << ": sorry: I don't know how to evaluate ExpName prefix parts." << endl;
	    return false;
      }

      const InterfacePort*gen = ent->find_generic(name_);
      if (gen) {
	    cerr << get_fileline() << ": sorry: I don't necessarily handle generic overrides." << endl;

	      // Evaluate the default expression and use that.
	    if (gen->expr)
		  return gen->expr->evaluate(ent, arc, val);
      }

      return evaluate(arc, val);
}
