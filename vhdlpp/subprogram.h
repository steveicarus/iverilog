#ifndef IVL_subprogram_H
#define IVL_subprogram_H
/*
 * Copyright (c) 2013-2021 Stephen Williams (steve@icarus.com)
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

# include  "StringHeap.h"
# include  "LineInfo.h"
# include  "scope.h"
# include  <iostream>
# include  <list>
# include  <cassert>

class InterfacePort;
class SequentialStmt;
class Package;
class VType;

class SubprogramBody : public LineInfo, public ScopeBase {

    public:
      SubprogramBody();
      ~SubprogramBody();

      const InterfacePort*find_param(perm_string nam) const;

      void set_statements(std::list<SequentialStmt*>*statements);
      inline bool empty_statements() const { return !statements_ || statements_->empty(); }

      int elaborate();
      int emit(std::ostream&out, Entity*ent, ScopeBase*scope);

	// Emit body as it would show up in a package.
      int emit_package(std::ostream&fd);

      void write_to_stream(std::ostream&fd) const;
      void dump(std::ostream&fd) const;

      const SubprogramHeader*header() const { return header_; }
      bool is_subprogram() const { return true; }

    private:
      std::list<SequentialStmt*>*statements_;
      SubprogramHeader*header_;

    friend class SubprogramHeader;
};

class SubprogramHeader : public LineInfo {
    public:
      SubprogramHeader(perm_string name, std::list<InterfacePort*>*ports,
		 const VType*return_type);
      virtual ~SubprogramHeader();

	// Return true if the specification (name, types, ports)
	// matches this subprogram and that subprogram.
      bool compare_specification(SubprogramHeader*that) const;

      int param_count() const { return ports_ ? ports_->size() : 0; }
      const InterfacePort*find_param(perm_string nam) const;
      const InterfacePort*peek_param(int idx) const;
      const VType*peek_param_type(int idx) const;
      const VType*peek_return_type() const { return return_type_; }

	// Computes the exact return type (e.g. std_logic_vector(7 downto 0)
	// instead of generic std_logic_vector)
      virtual const VType*exact_return_type(const std::vector<Expression*>&, Entity*, ScopeBase*);

      inline void set_package(const Package*pkg) { assert(!package_); package_ = pkg; }
      inline const Package*get_package() const { return package_; }

	// Checks if either return type or parameters are unbounded vectors.
      bool unbounded() const;

	// Is the subprogram coming from the standard library?
      virtual bool is_std() const { return false; }

      inline SubprogramBody*body() const { return body_; }
      void set_body(SubprogramBody*bdy);

      inline perm_string name() const { return name_; }

      int elaborate() { return (body_ ? body_->elaborate() : 0); }

	// Elaborates an argument basing on the types stored in the subprogram header.
      int elaborate_argument(Expression*expr, int idx, Entity*ent, ScopeBase*scope);

	// Emits the function name, including the package if required.
      int emit_full_name(const std::vector<Expression*>&argv,
                         std::ostream&out, Entity*, ScopeBase*) const;

	// Function name used in the emission step. The main purpose of this
	// method is to handle functions offered by standard VHDL libraries.
	// Allows to return different function names depending on the arguments
	// (think of size casting or signed/unsigned functions).
      virtual int emit_name(const std::vector<Expression*>&argv,
                            std::ostream&out, Entity*, ScopeBase*) const;

	// Emit arguments for a specific call. It allows to reorder or skip
	// some of the arguments if function signature is different in
	// SystemVerilog compared to VHDL.
      virtual int emit_args(const std::vector<Expression*>&argv,
                            std::ostream&out, Entity*, ScopeBase*) const;

	// Creates a new instance of the function that takes arguments of
	// a different type. It is used to allow VHDL functions that work with
	// unbounded std_logic_vectors, so there can be a separate instance
	// for limited length logic vector.
      SubprogramHeader*make_instance(std::vector<Expression*> arguments, ScopeBase*scope) const;

	// Emit header as it would show up in a package.
      int emit_package(std::ostream&fd) const;

      void write_to_stream(std::ostream&fd) const;
      void dump(std::ostream&fd) const;

    protected:
	// Tries to set the return type to a fixed type. VHDL functions that
	// return std_logic_vectors do not specify its length, as SystemVerilog
	// demands.
      void fix_return_type();

	// Procedure/function name
      perm_string name_;

      std::list<InterfacePort*>*ports_;
      const VType*return_type_;
      SubprogramBody*body_;
      const Package*package_;
};

// Class to define functions headers defined in the standard VHDL libraries.
class SubprogramStdHeader : public SubprogramHeader
{
    public:
      SubprogramStdHeader(perm_string nam, std::list<InterfacePort*>*ports,
              const VType*return_type) :
          SubprogramHeader(nam, ports, return_type) {}
      virtual ~SubprogramStdHeader() {};

      bool is_std() const { return true; }
};

// The simplest case, when only function name has to be changed.
class SubprogramBuiltin : public SubprogramStdHeader
{
    public:
      SubprogramBuiltin(perm_string vhdl_name, perm_string sv_name,
              std::list<InterfacePort*>*ports, const VType*return_type) :
          SubprogramStdHeader(vhdl_name, ports, return_type), sv_name_(sv_name) {}
      ~SubprogramBuiltin() {}

      int emit_name(const std::vector<Expression*>&, std::ostream&out, Entity*, ScopeBase*) const;

    private:
	// SystemVerilog counterpart function name
      perm_string sv_name_;
};

// Helper function to print out a human-readable function signature.
void emit_subprogram_sig(std::ostream&out, perm_string name,
        const std::list<const VType*>&arg_types);

#endif /* IVL_subprogram_H */
