/*
 * Copyright (c) 2011-2013 Stephen Williams (steve@icarus.com)
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

bool Expression::evaluate(ScopeBase*, int64_t&) const
{
      return false;
}

bool Expression::evaluate(Entity*, ScopeBase*scope, int64_t&val) const
{
      return evaluate(scope, val);
}

bool ExpArithmetic::evaluate(ScopeBase*scope, int64_t&val) const
{
      int64_t val1, val2;
      bool rc;

      rc = eval_operand1(scope, val1);
      if (rc == false)
	    return false;

      rc = eval_operand2(scope, val2);
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
	    return false;
	  case POW:
	    return false;
	  case xCONCAT: // not possible
	    return false;
      }

      return true;
}

bool ExpAttribute::evaluate(ScopeBase*scope, int64_t&val) const
{
	/* Special Case: The array attributes can sometimes be calculated all
	   the down to a literal integer at compile time, and all it
	   needs is the type of the base expression. (The base
	   expression doesn't even need to be evaluated.) */
      if (name_ == "length" || name_ == "right" || name_ == "left") {
	    const VType*base_type = base_->peek_type();

            if(!base_type) {
                const ExpName*name = NULL;

                if(scope && (name = dynamic_cast<const ExpName*>(base_))) {
                    const perm_string& n = name->peek_name();
                    if(const Variable*var = scope->find_variable(n))
                        base_type = var->peek_type();
                    else if(const Signal*sig = scope->find_signal(n))
                        base_type = sig->peek_type();
                    else if(const InterfacePort*port = scope->find_param(n))
                        base_type = port->type;
                }
            }

            if(!base_type)
                return false;      // I tried really hard, sorry

	    const VTypeArray*arr = dynamic_cast<const VTypeArray*>(base_type);
	    if (arr == 0) {
		  cerr << endl << get_fileline() << ": error: "
		       << "Cannot apply the '" << name_ << " attribute to non-array objects"
		       << endl;
                  ivl_assert(*this, false);
		  return false;
	    }

            if(name_ == "length") {
                int64_t size = arr->get_width(scope);

                if(size > 0)
                    val = size;
                else
                    return false;
            } else if(name_ == "left") {
		  arr->dimension(0).msb()->evaluate(scope, val);
            } else if(name_ == "right") {
		  arr->dimension(0).lsb()->evaluate(scope, val);
            } else ivl_assert(*this, false);

	    return true;
      }

      return false;
}

bool ExpAttribute::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
{
      if (!ent || !scope) {   // it's impossible to evaluate, probably it is inside a subprogram
            return false;
      }

      if (name_ == "left" || name_ == "right") {
	    const VType*base_type = base_->peek_type();
	    if (base_type == 0)
		  base_type = base_->probe_type(ent, scope);

	    ivl_assert(*this, base_type);

	    const VTypeArray*arr = dynamic_cast<const VTypeArray*>(base_type);
	    if (arr == 0) {
		  cerr << endl << get_fileline() << ": error: "
		       << "Cannot apply the '" << name_
		       << " attribute to non-array objects" << endl;
                  ivl_assert(*this, false);
		  return false;
	    }

	    ivl_assert(*this, arr->dimensions() == 1);
	    if(name_ == "left")
		  arr->dimension(0).msb()->evaluate(ent, scope, val);
	    else    // "right"
		  arr->dimension(0).lsb()->evaluate(ent, scope, val);

	    return true;
      }

      return evaluate(scope, val);
}

/*
 * I don't yet know how to evaluate concatenations. It is not likely
 * to come up anyhow.
 */
bool ExpConcat::evaluate(ScopeBase*, int64_t&) const
{
      return false;
}

bool ExpName::evaluate(ScopeBase*scope, int64_t&val) const
{
      const VType*type;
      Expression*exp;

      if (prefix_.get()) {
	    cerr << get_fileline() << ": sorry: I don't know how to evaluate ExpName prefix parts." << endl;
	    return false;
      }

      if (!scope)
	    return false;

      if (!scope->find_constant(name_, type, exp))
	    return false;

      return exp->evaluate(scope, val);
}

bool ExpName::evaluate(Entity*ent, ScopeBase*scope, int64_t&val) const
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
		  return gen->expr->evaluate(ent, scope, val);
      }

      return evaluate(scope, val);
}

bool ExpShift::evaluate(ScopeBase*scope, int64_t&val) const
{
      int64_t val1, val2;
      bool rc;

      rc = eval_operand1(scope, val1);
      if (rc == false)
	    return false;

      rc = eval_operand2(scope, val2);
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

bool ExpTime::evaluate(ScopeBase*, int64_t&val) const
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

bool ExpTime::evaluate(Entity*, ScopeBase*, int64_t&val) const
{
    return evaluate(NULL, NULL, val);
}
