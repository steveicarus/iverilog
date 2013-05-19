#ifndef __subprogram_H
#define __subprogram_H
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

# include  "StringHeap.h"
# include  "LineInfo.h"
# include  <iostream>
# include  <list>

class InterfacePort;
class SequentialStmt;
class VType;

class Subprogram : public LineInfo {

    public:
      Subprogram(perm_string name, std::list<InterfacePort*>*ports,
		 const VType*return_type);
      ~Subprogram();

      inline const perm_string&name() const { return name_; }

      void set_program_body(std::list<SequentialStmt*>*statements);

	// Return true if the specification (name, types, ports)
	// matches this subprogram and that subprogram.
      bool compare_specification(Subprogram*that) const;

	// Emit a definition as it would show up in a package.
      int emit_package(std::ostream&fd) const;

      void write_to_stream(std::ostream&fd) const;
      void dump(std::ostream&fd) const;

    private:
      perm_string name_;
      std::list<InterfacePort*>*ports_;
      const VType*return_type_;
      std::list<SequentialStmt*>*statements_;
};

#endif
