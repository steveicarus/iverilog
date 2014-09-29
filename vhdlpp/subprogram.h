#ifndef IVL_subprogram_H
#define IVL_subprogram_H
/*
 * Copyright (c) 2013-2014 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  "LineInfo.h"
# include  "scope.h"
# include  <iostream>
# include  <list>

class InterfacePort;
class SequentialStmt;
class VType;

class Subprogram : public LineInfo, public ScopeBase {

    public:
      Subprogram(perm_string name, std::list<InterfacePort*>*ports,
		 const VType*return_type);
      ~Subprogram();

      void set_parent(const ScopeBase*par);
      inline const ScopeBase*get_parent() const { return parent_; }

      inline const perm_string&name() const { return name_; }

      void set_program_body(std::list<SequentialStmt*>*statements);

	// Return true if the specification (name, types, ports)
	// matches this subprogram and that subprogram.
      bool compare_specification(Subprogram*that) const;

      int emit(ostream&out, Entity*ent, Architecture*arc);

	// Emit a definition as it would show up in a package.
      int emit_package(std::ostream&fd) const;

      void write_to_stream(std::ostream&fd) const;
      void dump(std::ostream&fd) const;

    private:
	// Determines appropriate return type. Un case of std_logic_vector
	// VHDL requires skipping its size in contrary to Verilog
      void fix_return_type(void);

      perm_string name_;
      const ScopeBase*parent_;
      std::list<InterfacePort*>*ports_;
      const VType*return_type_;
      std::list<SequentialStmt*>*statements_;
};

#endif /* IVL_subprogram_H */
