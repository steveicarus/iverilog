#ifndef __vtype_H
#define __vtype_H
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  <map>
# include  "StringHeap.h"

class VType {

    public:
      VType() { }
      virtual ~VType() =0;
};

/*
 * The global_types variable maps type names to a type
 * definition. This is after the "use" statements bring in the types
 * in included packages.
 */
extern std::map<perm_string, const VType*> global_types;

extern void preload_global_types(void);

/*
 * This class represents the primative types that are available to the
 * type subsystem.
 */
class VTypePrimitive : public VType {

    public:
      enum type_t { BOOLEAN, BIT, INTEGER, STDLOGIC };

    public:
      VTypePrimitive(type_t);
      ~VTypePrimitive();

      type_t type() const { return type_; }

    private:
      type_t type_;
};

extern const VTypePrimitive primitive_BOOLEAN;
extern const VTypePrimitive primitive_BIT;
extern const VTypePrimitive primitive_INTEGER;
extern const VTypePrimitive primitive_STDLOGIC;

class VTypeArray : public VType {

    public:
      VTypeArray(size_t dimensions, VType*etype);
      ~VTypeArray();

      size_t dimensions() const;
      VType* element_type() const;

    private:
      size_t dimensions_;
      VType*etype_;
};

#endif
