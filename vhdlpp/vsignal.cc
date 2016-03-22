/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2014
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

# include  "vsignal.h"
# include  "expression.h"
# include  "scope.h"
# include  "vtype.h"
# include  "std_types.h"
# include  <iostream>

using namespace std;

SigVarBase::SigVarBase(perm_string nam, const VType*typ, Expression*exp)
: name_(nam), type_(typ), init_expr_(exp), refcnt_sequ_(0)
{
}

SigVarBase::~SigVarBase()
{
}

void SigVarBase::elaborate(Entity*ent, ScopeBase*scope)
{
    if(init_expr_)
        init_expr_->elaborate_expr(ent, scope, peek_type());

    type_->elaborate(ent, scope);
}

void SigVarBase::type_elaborate_(VType::decl_t&decl)
{
      decl.type = type_;
}

int Signal::emit(ostream&out, Entity*ent, ScopeBase*scope, bool initialize)
{
      int errors = 0;

      VType::decl_t decl;
      type_elaborate_(decl);

      const VType*type = peek_type();
      if (peek_refcnt_sequ_() > 0
              || (!type->can_be_packed() && dynamic_cast<const VTypeArray*>(type)))
	    decl.reg_flag = true;
      errors += decl.emit(out, peek_name());

      const Expression*init_expr = peek_init_expr();
      if (initialize && init_expr) {
            /* Emit initialization value for wires as a weak assignment */
            if(!decl.reg_flag && !type->type_match(&primitive_REAL))
                out << ";" << endl << "/*init*/ assign (weak1, weak0) " << peek_name();

            out << " = ";
            init_expr->emit(out, ent, scope);
      }
      out << ";" << endl;
      return errors;
}

int Variable::emit(ostream&out, Entity*ent, ScopeBase*scope, bool initialize)
{
      int errors = 0;

      out << (!scope->is_subprogram() ? "static " : "automatic ");

      VType::decl_t decl;
      type_elaborate_(decl);
      decl.reg_flag = true;
      errors += decl.emit(out, peek_name());

      const Expression*init_expr = peek_init_expr();
      if (initialize && init_expr) {
	    out << " = ";
	    init_expr->emit(out, ent, scope);
      }
      out << ";" << endl;
      return errors;
}

void Variable::write_to_stream(std::ostream&fd)
{
      fd << "variable " << peek_name() << " : ";
      peek_type()->write_to_stream(fd);
      fd << ";" << endl;
}
