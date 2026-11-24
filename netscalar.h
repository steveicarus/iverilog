#ifndef IVL_netscalar_H
#define IVL_netscalar_H
/*
 * Copyright (c) 2013-2026 Stephen Williams (steve@icarus.com)
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

# include  "nettypes.h"
# include  "StringHeap.h"

class NetScope;

class netreal_t : public ivl_type_s {

    public:
      inline explicit netreal_t() { }
      ~netreal_t() override;

      ivl_variable_type_t base_type() const override;
      bool get_signed() const override { return true; }
      bool get_scalar() const override { return true; }

      std::ostream& debug_dump(std::ostream&) const override;

    public:
      static netreal_t type_real;
      static netreal_t type_shortreal;
};

class netstring_t : public ivl_type_s {

    public:
      inline explicit netstring_t() { }
      ~netstring_t() override;

      ivl_variable_type_t base_type() const override;

      std::ostream& debug_dump(std::ostream&) const override;

    public:
      static netstring_t type_string;
};

class netchandle_t : public ivl_type_s {

    public:
      inline explicit netchandle_t() { }
      ~netchandle_t() override;

      ivl_variable_type_t base_type() const override;

      std::ostream& debug_dump(std::ostream&) const override;

    public:
      static netchandle_t type_chandle;
};


/*
 * The type of a virtual interface variable (IEEE1800-2017 25.9). The
 * value of the variable is a handle to an instance of the named
 * interface, or null.
 */
class netvirtual_interface_t : public ivl_type_s {

    public:
      inline netvirtual_interface_t(perm_string name, NetScope*inst)
      : name_(name), instance_(inst) { }

      ivl_variable_type_t base_type() const override;

      inline perm_string name() const { return name_; }

	// An instance of the interface, to look up the members of the
	// interface. This is nil if the interface is not instantiated.
      inline NetScope* instance() const { return instance_; }

      std::ostream& debug_dump(std::ostream&) const override;

    private:
      bool test_equivalence(ivl_type_t that) const override;

      perm_string name_;
      NetScope*instance_;
};

#endif /* IVL_netscalar_H */
