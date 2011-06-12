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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "expression.h"
# include  "architec.h"
# include  "entity.h"
# include  "vsignal.h"
# include  <iostream>
# include  <typeinfo>
# include  <cassert>

using namespace std;

int Expression::elaborate_lval(Entity*, Architecture*)
{
      cerr << get_fileline() << ": error: Expression is not a valie l-value." << endl;
      return 1;
}

const VType* Expression::probe_type(Entity*, Architecture*) const
{
      return 0;
}

int ExpName::elaborate_lval(Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (const InterfacePort*cur = ent->find_port(name_)) {
	    if (cur->mode != PORT_OUT) {
		  cerr << get_fileline() << ": error: Assignment to "
			"input port " << name_ << "." << endl;
		  return errors += 1;
	    }

	    ent->set_declaration_l_value(name_, true);
	    set_type(cur->type);
	    return errors;
      }

      Signal*sig = arc->find_signal(name_);
      if (sig == 0) {
	    cerr << get_fileline() << ": error: Signal/variable " << name_
		 << " not found in this context." << endl;
	    return errors + 1;
      }

      set_type(sig->peek_type());
      return errors;
}

int ExpNameALL::elaborate_lval(Entity*ent, Architecture*arc)
{
      return Expression::elaborate_lval(ent, arc);
}

int Expression::elaborate_expr(Entity*, Architecture*, const VType*)
{
      cerr << get_fileline() << ": internal error: I don't know how to elaborate expression type=" << typeid(*this).name() << endl;
      return 1;
}

const VType* ExpBinary::probe_type(Entity*ent, Architecture*arc) const
{
      const VType*t1 = operand1_->probe_type(ent, arc);
      const VType*t2 = operand2_->probe_type(ent, arc);

      if (t1 == 0)
	    return t2;
      if (t2 == 0)
	    return t1;

      if (t1 == t2)
	    return t1;

      cerr << get_fileline() << ": internal error: I don't know how to resolve types of generic binary expressions." << endl;
      return 0;
}

int ExpBinary::elaborate_exprs(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      errors += operand1_->elaborate_expr(ent, arc, ltype);
      errors += operand2_->elaborate_expr(ent, arc, ltype);
      return errors;
}

int ExpArithmetic::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      assert(ltype != 0);
      errors += elaborate_exprs(ent, arc, ltype);
      return errors;
}

int ExpAttribute::elaborate_expr(Entity*ent, Architecture*arc, const VType*)
{
      int errors = 0;
      const VType*sub_type = base_->probe_type(ent, arc);
      errors += base_->elaborate_expr(ent, arc, sub_type);
      return errors;
}

int ExpBitstring::elaborate_expr(Entity*, Architecture*, const VType*)
{
      int errors = 0;
      return errors;
}

int ExpCharacter::elaborate_expr(Entity*, Architecture*, const VType*ltype)
{
      assert(ltype != 0);
      set_type(ltype);
      return 0;
}

const VType* ExpConditional::probe_type(Entity*, Architecture*) const
{
      return 0;
}

int ExpConditional::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0)
	    ltype = probe_type(ent, arc);

      assert(ltype);

      set_type(ltype);

	/* Note that the type for the condition expression need not
	   have anything to do with the type of this expression. */
      errors += cond_->elaborate_expr(ent, arc, 0);

      for (list<Expression*>::const_iterator cur = true_clause_.begin()
		 ; cur != true_clause_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, arc, ltype);
      }

      for (list<Expression*>::const_iterator cur = else_clause_.begin()
		 ; cur != else_clause_.end() ; ++cur) {
	    errors += (*cur)->elaborate_expr(ent, arc, ltype);
      }

      return errors;
}

int ExpLogical::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      assert(ltype != 0);
      errors += elaborate_exprs(ent, arc, ltype);
      return errors;
}

const VType* ExpName::probe_type(Entity*ent, Architecture*arc) const
{
      if (const InterfacePort*cur = ent->find_port(name_))
	    return cur->type;

      if (Signal*sig = arc->find_signal(name_))
	    return sig->peek_type();

      cerr << get_fileline() << ": error: Signal/variable " << name_
	   << " not found in this context." << endl;
      return 0;
}

int ExpName::elaborate_expr(Entity*, Architecture*, const VType*ltype)
{
      assert(ltype != 0);

      return 0;
}

const VType* ExpNameALL::probe_type(Entity*, Architecture*) const
{
      return 0;
}

const VType* ExpRelation::probe_type(Entity*ent, Architecture*arc) const
{
      const VType*type1 = peek_operand1()->probe_type(ent, arc);
      const VType*type2 = peek_operand2()->probe_type(ent, arc);

      if (type1 == type2)
	    return type1;

      if (type1 && !type2)
	    return type1;

      if (type2 && !type1)
	    return type2;

      cerr << get_fileline() << ": error: Type mismatch in relation expression." << endl;
      return type1;
}

int ExpRelation::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;

      if (ltype == 0) {
	    ltype = probe_type(ent, arc);
      }

      assert(ltype != 0);
      errors += elaborate_exprs(ent, arc, ltype);
      return errors;
}
