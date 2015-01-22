/*
 * Copyright (c) 2013-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "subprogram.h"
# include  "entity.h"
# include  "vtype.h"
# include  "sequential.h"
# include  "ivl_assert.h"
# include  "compiler.h"

using namespace std;

Subprogram::Subprogram(perm_string nam, list<InterfacePort*>*ports,
		       const VType*return_type)
: name_(nam), parent_(0), ports_(ports), return_type_(return_type), statements_(0)
{
}

Subprogram::~Subprogram()
{
}

void Subprogram::set_parent(const ScopeBase*par)
{
      ivl_assert(*this, parent_ == 0);
      parent_ = par;
}

void Subprogram::set_program_body(list<SequentialStmt*>*stmt)
{
      ivl_assert(*this, statements_==0);
      statements_ = stmt;
      fix_port_types();
}

// Functor used to add type casting to each return statement.
struct cast_return_type : public SeqStmtVisitor {
    cast_return_type(const VType*ret_type) : ret_(ret_type) {}

    void operator() (SequentialStmt*s)
    {
        ReturnStmt*ret;
        if((ret = dynamic_cast<ReturnStmt*>(s))) {
            ret->cast_to(ret_);
        }
    }

private:
    const VType*ret_;
};

void Subprogram::fix_port_types()
{
	// Check function parameters for unbounded vectors and possibly fix it.
      if(ports_) {
          for(std::list<InterfacePort*>::iterator it = ports_->begin();
                    it != ports_->end(); ++it) {
              check_unb_vector((*it)->type);
          }
      }

	// Try to settle at a fixed width return type.
      if(fixed_return_type())
          return;

	// Check if the returned type is an unbounded vector.
      if(check_unb_vector(return_type_)) {
          if(!statements_)
              return;

          // Go through the statement list and add type casting to return
          // statements to comply with the modified return type.
          for (std::list<SequentialStmt*>::iterator s = statements_->begin()
                ; s != statements_->end(); ++s) {
              cast_return_type r(return_type_);
              (*s)->visit(r);
          }
      }
}

bool Subprogram::check_unb_vector(const VType*&type)
{
    if(const VTypeArray*arr = dynamic_cast<const VTypeArray*>(type)) {
        if(arr->dimensions() == 1 && arr->dimension(0).is_box() ) {
            // For the time being, dynamic arrays work exclusively with vectors.
            // To emulate simple 'logic'/'bit' type, we need to create a vector
            // of width == 1, to be used as the array element type.
            // Effectively 'logic name []' becomes 'logic [0:0] name []'.
            Expression*zero = new ExpInteger(0);
            std::vector<VTypeArray::range_t> sub_range;
            sub_range.push_back(VTypeArray::range_t(zero, zero));
            VTypeArray*new_arr = new VTypeArray(arr, sub_range);
            type = get_global_typedef(new_arr);

            return true;
        }
    }

    return false;
}

bool Subprogram::compare_specification(Subprogram*that) const
{
      if (name_ != that->name_)
	    return false;

      if (return_type_==0) {
	    if (that->return_type_!=0)
		  return false;
      } else {
	    if (that->return_type_==0)
		  return false;

	    if (! return_type_->type_match(that->return_type_))
		  return false;
      }

      if (ports_==0) {
	    if (that->ports_!=0)
		  return false;

      } else {
	    if (that->ports_==0)
		  return false;

	    if (ports_->size() != that->ports_->size())
		  return false;
      }

      return true;
}

const InterfacePort*Subprogram::find_param(perm_string nam) const
{
      if(!ports_)
        return NULL;

      for (std::list<InterfacePort*>::const_iterator it = ports_->begin()
                ; it != ports_->end(); ++it) {
        if((*it)->name == nam)
            return *it;
      }

      return NULL;
}

const VType*Subprogram::peek_param_type(int idx) const
{
      if(!ports_ || idx < 0 || (size_t)idx >= ports_->size())
        return NULL;

      std::list<InterfacePort*>::const_iterator p = ports_->begin();
      std::advance(p, idx);

      return (*p)->type;
}

struct check_return_type : public SeqStmtVisitor {
    check_return_type(const Subprogram*subp) : subp_(subp), ret_type_(NULL) {}

    void operator() (SequentialStmt*s)
    {
        ReturnStmt*ret;
        if((ret = dynamic_cast<ReturnStmt*>(s))) {
            const Expression*expr = ret->peek_expr();
            const VType*t = NULL;

            if(const ExpName*n = dynamic_cast<const ExpName*>(expr)) {
                if(Variable*v = subp_->find_variable(n->peek_name()))
                    t = v->peek_type();
            } else {
                t = expr->peek_type();
            }

            if(!t) { // cannot determine the type at least in one case
                ret_type_ = NULL;
                return;
            }

            if(!ret_type_) { // this is first processed return statement
                ret_type_ = t;
            } else if(!t->type_match(ret_type_)) {
                // the function can return different types,
                // we cannot have fixed width
                ret_type_ = NULL;
                return;
            }
        }
    }

    const VType*get_type() const { return ret_type_; }

private:
    const Subprogram*subp_;
    const VType*ret_type_;
};

bool Subprogram::fixed_return_type(void)
{
    if(!statements_)
        return false;

    check_return_type r(this);

    for (std::list<SequentialStmt*>::iterator s = statements_->begin()
        ; s != statements_->end(); ++s) {
        (*s)->visit(r);
    }

    const VType*return_type = r.get_type();
    if(return_type && !return_type->is_unbounded()) {
        return_type_ = return_type;
        return true;
    } else {
        return false;
    }
}

void Subprogram::write_to_stream(ostream&fd) const
{
      fd << "  function " << name_ << "(";
      if (ports_ && ! ports_->empty()) {
	    list<InterfacePort*>::const_iterator cur = ports_->begin();
	    InterfacePort*curp = *cur;
	    fd << curp->name << " : ";
	    curp->type->write_to_stream(fd);
	    for (++cur ; cur != ports_->end() ; ++cur) {
		  curp = *cur;
		  fd << "; " << curp->name << " : ";
		  curp->type->write_to_stream(fd);
	    }
      }
      fd << ") return ";
      return_type_->write_to_stream(fd);
      fd << ";" << endl;
}
