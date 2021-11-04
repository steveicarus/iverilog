/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2015
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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

# include  "expression.h"
# include  "architec.h"
# include  <ivl_assert.h>
# include  <limits>
# include  <cmath>

using namespace std;

bool ExpArithmetic::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      int64_t val1, val2;
      bool rc;

      rc = eval_operand1(ent, scope, val1);
      if (rc == false)
	    return false;

      rc = eval_operand2(ent, scope, val2);
      if (rc == false)
	    return false;

      switch (fun_) {
	  case PLUS:
	    val = val1 + val2;
	    break;
	  case MINUS:
	    val = val1 - val2;
	    break;
	  case MULT:
	    val = val1 * val2;
	    break;
	  case DIV:
	    if (val2 == 0)
		  return false;
	    val = val1 / val2;
	    break;
	  case MOD:
	    if (val2 == 0)
		  return false;
	    val = val1 % val2;
	    break;
	  case REM:
	    if (val2 == 0)
		  return false;
	    val = val1 - (val1 / val2) * val2;
	    return false;
	  case POW:
	    val = (int64_t) pow(val1, val2);
	    break;
	  case xCONCAT: // not possible
	    return false;
      }

      return true;
}

bool ExpAttribute::test_array_type(const VType*type) const
{
      const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type);

      if (arr == 0) {
          cerr << endl << get_fileline() << ": error: "
              << "Cannot apply the '" << name_ << " attribute to non-array objects"
              << endl;
          ivl_assert(*this, false);
          return false;
      }

      if (arr->dimensions().size() > 1) {
          cerr << endl << get_fileline() << ": error: "
              << "Cannot apply the '" << name_
              << " attribute to multidimensional arrays" << endl;
          return false;
      }

      if (arr->dimension(0).is_box())
          return false;

      return true;
}

bool ExpAttribute::evaluate_type_attr(const VType*type, Entity*ent, ScopeBase*scope, int64_t&val) const
{
      if (name_ == "length" && test_array_type(type)) {
          int64_t size = type->get_width(scope);

          if(size > 0) {
              val = size;
              return true;
          }
      } else if (name_ == "left" && test_array_type(type)) {
          const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type);
          return arr->dimension(0).msb()->evaluate(ent, scope, val);
      } else if (name_ == "right" && test_array_type(type)) {
          const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type);
          return arr->dimension(0).lsb()->evaluate(ent, scope, val);
      }

      return false;
}

bool ExpObjAttribute::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      const VType*base_type = base_->peek_type();

      if (base_type == NULL)
          base_type = base_->probe_type(ent, scope);

      if (base_type)
          return evaluate_type_attr(base_type, ent, scope, val);

      return false;
}

bool ExpTypeAttribute::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      return evaluate_type_attr(base_, ent, scope, val);
}

bool ExpName::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      if (prefix_.get()) {
          cerr << get_fileline() << ": sorry: I don't know how to evaluate "
               << "ExpName prefix parts." << endl;
          return false;
      }

      if (scope) {
          const VType*type;
          Expression*exp;

          if (scope->find_constant(name_, type, exp))
              return exp->evaluate(ent, scope, val);
      }

      return false;
}

bool ExpShift::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      int64_t val1, val2;
      bool rc;

      rc = eval_operand1(ent, scope, val1);
      if (rc == false)
	    return false;

      rc = eval_operand2(ent, scope, val2);
      if (rc == false)
	    return false;

      switch (shift_) {
	  case SRL:
	    val = (uint64_t)val1 >> (uint64_t)val2;
	    break;
	  case SLL:
	    val = (uint64_t)val1 << (uint64_t)val2;
	    break;
	  case SRA:
	    val = (int64_t)val1 >> (int64_t)val2;
	    break;
	  case SLA:
	    val = (int64_t)val1 << (int64_t)val2;
	    break;
	  case ROR:
	  case ROL:
	    return false;
      }

      return true;
}

/*bool ExpTime::evaluate(Entity*, ScopeBase*, int64_t&val) const
{
    double v = to_fs();

    if(v > std::numeric_limits<int64_t>::max()) {
        val = std::numeric_limits<int64_t>::max();
        cerr << get_fileline() << ": sorry: Time value is higher than the "
             << "handled limit, reduced to " << val << " fs." << endl;
    }

    val = v;
    return true;
}
}*/
