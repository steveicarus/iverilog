/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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
# include  "sequential.h"
# include  "vtype.h"
# include  "package.h"
# include  <iostream>

using namespace std;

int SubprogramBody::emit_package(ostream&fd)
{
      int errors = 0;

      for (map<perm_string,Variable*>::const_iterator cur = new_variables_.begin()
         ; cur != new_variables_.end() ; ++cur) {
          // Enable reg_flag for variables
          cur->second->count_ref_sequ();
          errors += cur->second->emit(fd, NULL, this, false);
      }

    // Emulate automatic functions (add explicit initial value assignments)
      for (map<perm_string,Variable*>::const_iterator cur = new_variables_.begin()
         ; cur != new_variables_.end() ; ++cur) {
          Variable*var = cur->second;

          if(const Expression*init = var->peek_init_expr()) {
              fd << cur->first << " = ";
              init->emit(fd, NULL, this);
              fd << "; // automatic function emulation" << endl;
          }
      }

      if (statements_) {
	    for (list<SequentialStmt*>::const_iterator cur = statements_->begin()
		       ; cur != statements_->end() ; ++cur) {
		  errors += (*cur)->emit(fd, NULL, const_cast<SubprogramBody*>(this));
	    }
      } else {
	    fd << " begin /* empty body */ end" << endl;
      }

      return errors;
}

int SubprogramHeader::emit_package(ostream&fd) const
{
      int errors = 0;

      if (return_type_) {
	    fd << "function automatic ";
	    return_type_->emit_def(fd, empty_perm_string);
      } else {
	    fd << "task automatic";
      }

      fd << " \\" << name_ << " (";

      for (list<InterfacePort*>::const_iterator cur = ports_->begin()
		 ; cur != ports_->end() ; ++cur) {
	    if (cur != ports_->begin())
		  fd << ", ";
	    InterfacePort*curp = *cur;
	    switch (curp->mode) {
		case PORT_IN:
		  fd << "input ";
		  break;
		case PORT_OUT:
		  fd << "output ";
		  break;
		case PORT_INOUT:
		  fd << "inout ";
		  break;
		case PORT_NONE:
		  fd << "inout /* PORT_NONE? */ ";
		  break;
	    }

	    errors += curp->type->emit_def(fd, curp->name);
      }

      fd << ");" << endl;

      if (body_)
          body_->emit_package(fd);

      if (return_type_)
	    fd << "endfunction" << endl;
      else
	    fd << "endtask" << endl;

      return errors;
}

int SubprogramHeader::emit_full_name(const std::vector<Expression*>&argv,
                        std::ostream&out, Entity*ent, ScopeBase*scope) const
{
    // If this function has an elaborated definition, and if
    // that definition is in a package, then include the
    // package name as a scope qualifier. This assures that
    // the SV elaborator finds the correct VHDL elaborated
    // definition. It should not be emitted only if we call another
    // function from the same package.
    const SubprogramBody*subp = dynamic_cast<const SubprogramBody*>(scope);
    if (package_ && (!subp || !subp->header() || subp->header()->get_package() != package_))
        out << "\\" << package_->name() << " ::";

    return emit_name(argv, out, ent, scope);
}

int SubprogramHeader::emit_name(const std::vector<Expression*>&,
                                std::ostream&out, Entity*, ScopeBase*) const
{
    out << "\\" << name_;
    return 0;
}

int SubprogramHeader::emit_args(const std::vector<Expression*>&argv,
                                std::ostream&out, Entity*ent, ScopeBase*scope) const
{
      int errors = 0;

      for (size_t idx = 0; idx < argv.size() ; idx += 1) {
          if (idx > 0) out << ", ";
          errors += argv[idx]->emit(out, ent, scope);
      }

      return errors;
}

int SubprogramBuiltin::emit_name(const std::vector<Expression*>&,
                                 std::ostream&out, Entity*, ScopeBase*) const
{
    // do not escape the names for builtin functions
    out << sv_name_;
    return 0;
}

void emit_subprogram_sig(ostream&out, perm_string name,
        const list<const VType*>&arg_types)
{
    out << name << "(";
    bool first = true;
    for(list<const VType*>::const_iterator it = arg_types.begin();
            it != arg_types.end(); ++it) {
        if(first)
            first = false;
        else
            out << ", ";

        if(*it)
            (*it)->write_to_stream(out);
        else
            out << "<unresolved type>";
    }
    out << ")";
}
