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
# include  <list>
# include  <vector>
# include  <climits>
# include  <inttypes.h>
# include  "StringHeap.h"

class Expression;
class prange_t;

/*
 * A description of a VHDL type consists of a graph of VType
 * objects. Derived types are specific kinds of types, and those that
 * are compound may in turn reference other types.
 */
class VType {

    public:
      VType() { }
      virtual ~VType() =0;

	// This virtual method writes a VHDL-accurate representation
	// of this type to the designated stream. This is used for
	// writing parsed types to library files.
      virtual void write_to_stream(std::ostream&fd) const;

	// This virtual method writes a human-readable version of the
	// type to a given file for debug purposes. (Question: is this
	// really necessary given the write_to_stream method?)
      virtual void show(std::ostream&) const;

	// This virtual method emits a definition for the specific
	// type. It is used to emit typedef's.
      virtual int emit_def(std::ostream&out, perm_string name) const =0;

    private:
      friend class decl_t;
	// This virtual method is called to emit the declaration. This
	// is used by the decl_t object to emit variable/wire/port declarations.
      virtual int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const =0;

    public:
	// A couple places use the VType along with a few
	// per-declaration details, so provide a common structure for
	// holding that stuff together.
      struct decl_t {
	    decl_t() : type(0), reg_flag(false) { }
	    int emit(std::ostream&out, perm_string name) const;

	    const VType*type;
	    bool reg_flag;
      };

};

inline std::ostream&operator << (std::ostream&out, const VType&item)
{
      item.show(out);
      return out;
}

extern void preload_global_types(void);

/*
 * This class represents the primitive types that are available to the
 * type subsystem.
 */
class VTypePrimitive : public VType {

    public:
      enum type_t { BOOLEAN, BIT, INTEGER, STDLOGIC };

    public:
      VTypePrimitive(type_t);
      ~VTypePrimitive();

      void write_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;

      type_t type() const { return type_; }

      int emit_primitive_type(std::ostream&fd) const;

      int emit_def(std::ostream&out, perm_string name) const;
    private:
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

    private:
      type_t type_;
};

extern const VTypePrimitive* primitive_BOOLEAN;
extern const VTypePrimitive* primitive_BIT;
extern const VTypePrimitive* primitive_INTEGER;
extern const VTypePrimitive* primitive_STDLOGIC;

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
	    range_t() : msb_(0), lsb_(0) { }
	    range_t(Expression*m, Expression*l) : msb_(m), lsb_(l) { }

	    bool is_box() const { return msb_==0 && lsb_==0; }

	    Expression* msb() const { return msb_; }
	    Expression* lsb() const { return lsb_; }

	  private:
	    Expression* msb_;
	    Expression* lsb_;
      };

    public:
      VTypeArray(const VType*etype, const std::vector<range_t>&r, bool signed_vector =false);
      VTypeArray(const VType*etype, std::list<prange_t*>*r, bool signed_vector =false);
      ~VTypeArray();

      void write_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;

      size_t dimensions() const;
      const range_t&dimension(size_t idx) const
      { return ranges_[idx]; }

      bool signed_vector() const { return signed_flag_; }

      const VType* element_type() const;

      int emit_def(std::ostream&out, perm_string name) const;

    private:
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

    private:
      const VType*etype_;

      std::vector<range_t> ranges_;
      bool signed_flag_;
};

class VTypeRange : public VType {

    public:
      VTypeRange(const VType*base, int64_t max_val, int64_t min_val);
      ~VTypeRange();

      void write_to_stream(std::ostream&fd) const;

      int emit_def(std::ostream&out, perm_string name) const;
    private:
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

    private:
      const VType*base_;
      int64_t max_, min_;
};

class VTypeEnum : public VType {

    public:
      VTypeEnum(const std::list<perm_string>*names);
      ~VTypeEnum();

      void show(std::ostream&) const;

      int emit_def(std::ostream&out, perm_string name) const;
    private:
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

    private:
      std::vector<perm_string>names_;
};

class VTypeRecord : public VType {

    public:
      class element_t {
	  public:
	    element_t(perm_string name, const VType*type);

	    void write_to_stream(std::ostream&) const;

	    inline perm_string peek_name() const { return name_; }
	    inline const VType* peek_type() const { return type_; }

	  private:
	    perm_string name_;
	    const VType*type_;

	  private:// Not implement
	    element_t(const element_t&);
	    element_t& operator= (const element_t);
      };

    public:
      explicit VTypeRecord(std::list<element_t*>*elements);
      ~VTypeRecord();

      void write_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;

      int emit_def(std::ostream&out, perm_string name) const;

      const element_t* element_by_name(perm_string name) const;

    private:
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

    private:
      std::vector<element_t*> elements_;
};

class VTypeDef : public VType {

    public:
      VTypeDef(perm_string name, const VType*is);
      ~VTypeDef();

      int emit_typedef(std::ostream&out) const;

      int emit_def(std::ostream&out, perm_string name) const;
    private:
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

    private:
      perm_string name_;
      const VType*type_;
};

#endif
