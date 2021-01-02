/*
 *  VHDL variable and signal types.
 *
 *  Copyright (C) 2008-2021  Nick Gasson (nick@nickg.me.uk)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef INC_VHDL_TYPE_HH
#define INC_VHDL_TYPE_HH

#include "vhdl_element.hh"

enum vhdl_type_name_t {
   VHDL_TYPE_STD_LOGIC,
   VHDL_TYPE_STD_ULOGIC,
   VHDL_TYPE_STD_LOGIC_VECTOR,
   VHDL_TYPE_STRING,
   VHDL_TYPE_LINE,
   VHDL_TYPE_FILE,
   VHDL_TYPE_INTEGER,
   VHDL_TYPE_BOOLEAN,
   VHDL_TYPE_SIGNED,
   VHDL_TYPE_UNSIGNED,
   VHDL_TYPE_TIME,
   VHDL_TYPE_ARRAY
};

/*
 * A type at the moment is just a name. It shouldn't get
 * too much more complex, as Verilog's type system is much
 * simpler than VHDL's.
 */
class vhdl_type : public vhdl_element {
public:
   // Scalar constructor
   explicit vhdl_type(vhdl_type_name_t name, int msb = 0, int lsb = 0)
      : name_(name), msb_(msb), lsb_(lsb), base_(NULL) {}

   // Array constructor
   vhdl_type(vhdl_type *base, const std::string &array_name,
             int msb, int lsb)
      : name_(VHDL_TYPE_ARRAY), msb_(msb), lsb_(lsb), base_(base),
        array_name_(array_name) {}

   // Copy constructor
   vhdl_type(const vhdl_type &other);

   virtual ~vhdl_type();

   void emit(std::ostream &of, int level) const;
   vhdl_type_name_t get_name() const { return name_; }
   std::string get_string() const;
   std::string get_decl_string() const;
   std::string get_type_decl_string() const;
   vhdl_type *get_base() const;
   int get_width() const { return msb_ - lsb_ + 1; }
   int get_msb() const { return msb_; }
   int get_lsb() const { return lsb_; }

   // Common types
   static vhdl_type *std_logic();
   static vhdl_type *std_ulogic();
   static vhdl_type *string();
   static vhdl_type *line();
   static vhdl_type *std_logic_vector(int msb, int lsb);
   static vhdl_type *nunsigned(int width, int lsb=0);
   static vhdl_type *nsigned(int width, int lsb=0);
   static vhdl_type *integer();
   static vhdl_type *boolean();
   static vhdl_type *time();

   static vhdl_type *type_for(int width, bool issigned,
                              int lsb=0, bool unresolved=false);
   static vhdl_type *array_of(vhdl_type *b, const std::string &n, int m, int l);
protected:
   vhdl_type_name_t name_;
   int msb_, lsb_;
   vhdl_type *base_;   // Array base type for VHDL_TYPE_ARRAY
   std::string array_name_; // Type name for the array `type array_name_ is ...'
};

#endif
