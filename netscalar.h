#ifndef IVL_netscalar_H
#define IVL_netscalar_H
/*
 * Copyright (c) 2013-2014 Stephen Williams (steve@icarus.com)
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

class netreal_t : public ivl_type_s {

    public:
      inline explicit netreal_t() { }
      ~netreal_t();

      ivl_variable_type_t base_type() const;
      bool get_signed() const { return true; }
      bool get_scalar() const { return true; }

      std::ostream& debug_dump(std::ostream&) const;

    public:
      static netreal_t type_real;
      static netreal_t type_shortreal;
};

class netstring_t : public ivl_type_s {

    public:
      inline explicit netstring_t() { }
      ~netstring_t();

      ivl_variable_type_t base_type() const;

      std::ostream& debug_dump(std::ostream&) const;

    public:
      static netstring_t type_string;
};

#endif /* IVL_netscalar_H */
