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

# include  <iostream>
# include  <map>
# include  <vector>
# include  <climits>
# include  "StringHeap.h"

/*
 * A description of a VHDL type consists of a graph of VType
 * objects. Derived types are specific kinds of types, and those that
 * are compound may in turn reference other types.
 */
class VType {

    public:
      VType() { }
      virtual ~VType() =0;

      virtual void show(std::ostream&) const;

    public:
      enum vtype_t { VNONE, VBOOL, VLOGIC };
      struct decl_t {
	  public:
	    decl_t() : signed_flag(false), type(VNONE), msb(0), lsb(0) { }
	    int emit(std::ostream&out, perm_string name) const;
	  public:
	    bool signed_flag;
	    vtype_t type;
	    long msb, lsb;
      };
      virtual void elaborate(decl_t&decl) const =0;
};

inline std::ostream&operator << (std::ostream&out, const VType&item)
{
      item.show(out);
      return out;
}

/*
 * The global_types variable maps type names to a type
 * definition. This is after the "use" statements bring in the types
 * in included packages.
 */
extern std::map<perm_string, const VType*> global_types;

extern void preload_global_types(void);

extern void import_ieee(void);
extern void import_ieee_use(perm_string package, perm_string name);

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

      void show(std::ostream&) const;
      void elaborate(decl_t&decl) const;

      type_t type() const { return type_; }

    private:
      type_t type_;
};

extern const VTypePrimitive primitive_BOOLEAN;
extern const VTypePrimitive primitive_BIT;
extern const VTypePrimitive primitive_INTEGER;
extern const VTypePrimitive primitive_STDLOGIC;

/*
 * An array is a compound N-dimensional array of element type. The
 * construction of the array is from an element type and a vector of
 * ranges. The array type can be left incomplete by leaving some
 * ranges as "box" ranges, meaning present but not defined.
 */
class VTypeArray : public VType {

    public:
      class range_t {
	  public:
	    range_t()             : msb_(INT_MAX), lsb_(INT_MIN) { }
	    range_t(int m, int l) : msb_(m),       lsb_(l)       { }

	    bool is_box() const { return msb_==INT_MAX && lsb_==INT_MIN; }

	    int msb() const { return msb_; }
	    int lsb() const { return lsb_; }

	  private:
	    int msb_;
	    int lsb_;
      };

    public:
      VTypeArray(const VType*etype, const std::vector<range_t>&r, bool signed_vector =false);
      ~VTypeArray();

      void show(std::ostream&) const;
      void elaborate(decl_t&decl) const;

      size_t dimensions() const;
      const range_t&dimension(size_t idx) const
      { return ranges_[idx]; }

      bool signed_vector() const { return signed_flag_; }

      const VType* element_type() const;

    private:
      const VType*etype_;

      std::vector<range_t> ranges_;
      bool signed_flag_;
};

#endif
