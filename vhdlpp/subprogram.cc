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
# include  <cassert>

using namespace std;

SubprogramBody::SubprogramBody()
    : statements_(NULL), header_(NULL)
{
}

SubprogramBody::~SubprogramBody()
{
}

const InterfacePort*SubprogramBody::find_param(perm_string nam) const
{
      if(!header_)
          return NULL;

      return header_->find_param(nam);
}

void SubprogramBody::set_statements(list<SequentialStmt*>*stmt)
{
      ivl_assert(*this, statements_==0);
      statements_ = stmt;
}

int SubprogramBody::elaborate()
{
    int errors = 0;

    for (list<SequentialStmt*>::const_iterator cur = statements_->begin()
                ; cur != statements_->end() ; ++cur) {
            errors += (*cur)->elaborate(0, this);
    }

    return errors;
}

void SubprogramBody::write_to_stream(ostream&fd) const
{
      for (map<perm_string,Variable*>::const_iterator cur = new_variables_.begin()
         ; cur != new_variables_.end() ; ++cur) {
            cur->second->write_to_stream(fd);
      }

      fd << "begin" << endl;

      if (statements_) {
            for (list<SequentialStmt*>::const_iterator cur = statements_->begin()
                       ; cur != statements_->end() ; ++cur) {
                  (*cur)->write_to_stream(fd);
            }
      } else {
	    fd << "--empty body" << endl;
      }

      fd << "end function " << header_->name() << ";" << endl;
}

SubprogramHeader::SubprogramHeader(perm_string nam, list<InterfacePort*>*ports,
		       const VType*return_type)
: name_(nam), ports_(ports), return_type_(return_type), body_(NULL), package_(NULL)
{
}

SubprogramHeader::~SubprogramHeader()
{
    delete body_;

    if(ports_) {
        for(list<InterfacePort*>::iterator it = ports_->begin();
                it != ports_->end(); ++it)
        {
            delete *it;
        }
        delete ports_;
    }
}

bool SubprogramHeader::compare_specification(SubprogramHeader*that) const
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

const InterfacePort*SubprogramHeader::find_param(perm_string nam) const
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

const InterfacePort*SubprogramHeader::peek_param(int idx) const
{
      if(!ports_ || idx < 0 || (size_t)idx >= ports_->size())
        return NULL;

      std::list<InterfacePort*>::const_iterator p = ports_->begin();
      std::advance(p, idx);

      return *p;
}

const VType*SubprogramHeader::peek_param_type(int idx) const
{
      const InterfacePort*port = peek_param(idx);

      if(port)
        return port->type;

      return NULL;
}

const VType*SubprogramHeader::exact_return_type(const std::vector<Expression*>&argv, Entity*ent, ScopeBase*scope)
{
    const VTypeArray*orig_ret = dynamic_cast<const VTypeArray*>(return_type_);

    if(!orig_ret)
        return return_type_;

    const VTypeArray*arg = dynamic_cast<const VTypeArray*>(argv[0]->fit_type(ent, scope, orig_ret));

    if(!arg)
        return return_type_;

    VTypeArray*ret = new VTypeArray(orig_ret->element_type(), arg->dimensions(), orig_ret->signed_vector());
    ret->set_parent_type(orig_ret);

    return ret;
}

bool SubprogramHeader::unbounded() const {
    if(return_type_ && return_type_->is_unbounded())
       return true;

    if(ports_) {
        for(std::list<InterfacePort*>::const_iterator it = ports_->begin();
                it != ports_->end(); ++it) {
            if((*it)->type->is_unbounded())
                return true;
        }
    }

    return false;
}

void SubprogramHeader::set_body(SubprogramBody*bdy)
{
    ivl_assert(*this, !body_);
    body_ = bdy;
    ivl_assert(*this, !bdy->header_);
    bdy->header_ = this;
}

int SubprogramHeader::elaborate_argument(Expression*expr, int idx,
                                         Entity*ent, ScopeBase*scope)
{
    const VType*type = expr->probe_type(ent, scope);
    const InterfacePort*param = peek_param(idx);

    if(!param) {
        cerr << expr->get_fileline()
                << ": error: Too many arguments when calling "
                << name_ << "." << endl;
        return 1;
    }

    // Enable reg_flag for variables that might be modified in subprograms
    if(param->mode == PORT_OUT || param->mode == PORT_INOUT) {
        if(const ExpName*e = dynamic_cast<const ExpName*>(expr)) {
            if(Signal*sig = scope->find_signal(e->peek_name()))
                sig->count_ref_sequ();
            else if(Variable*var = scope->find_variable(e->peek_name()))
                var->count_ref_sequ();
        }
    }

    if(!type)
        type = param->type;

    return expr->elaborate_expr(ent, scope, type);
}

SubprogramHeader*SubprogramHeader::make_instance(std::vector<Expression*> arguments,
                                                 ScopeBase*scope) const {
    assert(arguments.size() == ports_->size());

    std::list<InterfacePort*>*ports = new std::list<InterfacePort*>;
    int i = 0;

    // Change the argument types to match the ones that were used during
    // the function call
    for(std::list<InterfacePort*>::iterator it = ports_->begin();
            it != ports_->end(); ++it) {
        InterfacePort*p = new InterfacePort(**it);
        p->type = arguments[i++]->peek_type()->clone();
        assert(p->type);
        ports->push_back(p);
    }

    char buf[80];
    snprintf(buf, sizeof(buf), "__%s_%p", name_.str(), ports);
    perm_string new_name = lex_strings.make(buf);
    SubprogramHeader*instance = new SubprogramHeader(new_name, ports, return_type_);

    if(body_) {
        SubprogramBody*body_inst = new SubprogramBody();

        // Copy variables
        for(std::map<perm_string,Variable*>::iterator it = body_->new_variables_.begin();
                it != body_->new_variables_.end(); ++it) {
            Variable*v = new Variable(it->first, it->second->peek_type()->clone());
            body_inst->new_variables_[it->first] = v;
        }

        body_inst->set_statements(body_->statements_);
        instance->set_package(package_);
        instance->set_body(body_inst);
        instance->fix_return_type();
    }

    scope->bind_subprogram(new_name, instance);

    return instance;
}

struct check_return_type : public SeqStmtVisitor {
    explicit check_return_type(const SubprogramBody*subp) : subp_(subp), ret_type_(NULL) {}

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
    const SubprogramBody*subp_;
    const VType*ret_type_;
};

void SubprogramHeader::fix_return_type()
{
    if(!body_ || !body_->statements_)
        return;

    check_return_type r(body_);

    for (std::list<SequentialStmt*>::iterator s = body_->statements_->begin()
        ; s != body_->statements_->end(); ++s) {
        (*s)->visit(r);
    }

    VType*return_type = const_cast<VType*>(r.get_type());
    if(return_type && !return_type->is_unbounded()) {
        // Let's check if the variable length can be evaluated without any scope.
        // If not, then it depends on information about e.g. function params
        if(return_type->is_variable_length(NULL)) {
            if(VTypeArray*arr = dynamic_cast<VTypeArray*>(return_type))
                arr->evaluate_ranges(body_);
        }
        return_type_ = return_type;
    }
}

void SubprogramHeader::write_to_stream(ostream&fd) const
{
      if(return_type_)
	    fd << "function ";
      else
	    fd << "procedure ";

      fd << name_;
      if (ports_ && ! ports_->empty()) {
	    fd << "(";
	    list<InterfacePort*>::const_iterator cur = ports_->begin();
	    InterfacePort*curp = *cur;
	    fd << curp->name << " : ";
	    curp->type->write_to_stream(fd);
	    for (++cur ; cur != ports_->end() ; ++cur) {
		  curp = *cur;
		  fd << "; " << curp->name << " : ";
		  curp->type->write_to_stream(fd);
	    }
	    fd << ")";
      }

      if( return_type_) {
	    fd << " return ";
	    return_type_->write_to_stream(fd);
      }
}
