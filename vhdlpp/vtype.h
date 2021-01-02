#ifndef IVL_vtype_H
#define IVL_vtype_H
/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2014 / Stephen Williams (steve@icarus.com),
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

# include  <iostream>
# include  <list>
# include  <map>
# include  <vector>
# include  <climits>
# include  <inttypes.h>
# include  "StringHeap.h"

class Architecture;
class ScopeBase;
class Entity;
class Expression;
class ExpRange;
class VTypeDef;
class ScopeBase;

typedef enum typedef_topo_e { NONE=0, PENDING, MARKED } typedef_topo_t;
typedef std::map<const VTypeDef*, typedef_topo_t> typedef_context_t;

/*
 * A description of a VHDL type consists of a graph of VType
 * objects. Derived types are specific kinds of types, and those that
 * are compound may in turn reference other types.
 */
class VType {

    public:
      VType() { }
      virtual ~VType() =0;

      virtual VType*clone() const =0;

	// This is rarely used, but some types may have expressions
	// that need to be elaborated.
      virtual int elaborate(Entity*end, ScopeBase*scope) const;

	// This virtual method returns true if that is equivalent to
	// this type. This method is used for example to compare
	// function prototypes.
      virtual bool type_match(const VType*that) const;

	// This virtual method writes a VHDL-accurate representation
	// of this type to the designated stream. This is used for
	// writing parsed types to library files.
      virtual void write_to_stream(std::ostream&fd) const;

	// This is like the above, but is the root function called
	// directly after the "type <name> is..." when writing type
	// definitions. Most types accept the default definition of this.
      virtual void write_type_to_stream(std::ostream&fd) const;

	// Emits a type definition. This is used to distinguish types and
	// subtypes.
      virtual void write_typedef_to_stream(std::ostream&fd, perm_string name) const;

	// This virtual method writes a human-readable version of the
	// type to a given file for debug purposes. (Question: is this
	// really necessary given the write_to_stream method?)
      virtual void show(std::ostream&) const;

	// This virtual method emits a definition for the specific
	// type. It is used to emit typedef's.
      virtual int emit_def(std::ostream&out, perm_string name) const =0;

	// This virtual method causes VTypeDef types to emit typedefs
	// of themselves. The VTypeDef implementation of this method
	// uses this method recursively to do a depth-first emit of
	// all the types that it emits.
      virtual int emit_typedef(std::ostream&out, typedef_context_t&ctx) const;

	// Determines if a type can be used in Verilog packed array.
      virtual bool can_be_packed() const { return false; }

	// Returns true if the type has an undefined dimension.
      virtual bool is_unbounded() const { return false; }

	// Checks if the variable length is dependent on other expressions, that
	// cannot be evaluated (e.g. 'length, 'left, 'right).
      virtual bool is_variable_length(ScopeBase*) const { return false; }

	// Returns a perm_string that can be used in automatically created
	// typedefs (i.e. not ones defined by the user).
      perm_string get_generic_typename() const;

	// Returns the type width in bits or negative number if it is impossible
	// to evaluate.
      virtual int get_width(ScopeBase*) const { return -1; }

	// This virtual method is called to emit the declaration. This
	// is used by the decl_t object to emit variable/wire/port declarations.
      virtual int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

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

    protected:
      inline void emit_name(std::ostream&out, perm_string name) const
      {
        if(name != empty_perm_string)
            out << " \\" << name << " ";
      }
};

inline std::ostream&operator << (std::ostream&out, const VType&item)
{
      item.show(out);
      return out;
}

extern void preload_global_types(void);

/*
 * This type is a placeholder for ERROR types.
 */
class VTypeERROR : public VType {
    VType*clone() const { return NULL; }

    public:
      int emit_def(std::ostream&out, perm_string name) const;
};

/*
 * This class represents the primitive types that are available to the
 * type subsystem.
 */
class VTypePrimitive : public VType {

    public:
      enum type_t { BIT, INTEGER, NATURAL, REAL, STDLOGIC, TIME };

    public:
      explicit VTypePrimitive(type_t tt, bool packed = false);
      ~VTypePrimitive();

      VType*clone() const { return new VTypePrimitive(*this); }

      bool type_match(const VType*that) const;
      void write_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;
      int get_width(ScopeBase*scope) const;

      type_t type() const { return type_; }

      int emit_primitive_type(std::ostream&fd) const;
      int emit_def(std::ostream&out, perm_string name) const;

      bool can_be_packed() const { return packed_; }

    private:
      type_t type_;
      bool packed_;
};

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
	    range_t(Expression*m = NULL, Expression*l = NULL, bool down_to = true) :
                msb_(m), lsb_(l), direction_(down_to) { }

	    range_t*clone() const;

	    inline bool is_box() const { return msb_==0 && lsb_==0; }
	    inline bool is_downto() const { return direction_; }

	    inline Expression* msb() const { return msb_; }
	    inline Expression* lsb() const { return lsb_; }

	  private:
	    Expression* msb_;
	    Expression* lsb_;
	    bool direction_;
      };

    public:
      VTypeArray(const VType*etype, const std::vector<range_t>&r, bool signed_vector = false);
      VTypeArray(const VType*etype, std::list<ExpRange*>*r, bool signed_vector = false);
      VTypeArray(const VType*etype, int msb, int lsb, bool signed_vector = false);
      ~VTypeArray();

      VType*clone() const;

      int elaborate(Entity*ent, ScopeBase*scope) const;
      bool type_match(const VType*that) const;
      void write_to_stream(std::ostream&fd) const;
      void write_type_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;
      int get_width(ScopeBase*scope) const;

      const std::vector<range_t>&dimensions() const { return ranges_; };
      const range_t&dimension(size_t idx) const
      { return ranges_[idx]; }

      inline bool signed_vector() const { return signed_flag_; }

	// returns the type of element held in the array
      inline const VType* element_type() const { return parent_ ? parent_->element_type() : etype_; }

	// returns the basic type of element held in the array
	// (unfolds typedefs and multidimensional arrays)
	// typedef_allowed decides if VTypeDef can be returned or should
	// it be unfolded
      const VType* basic_type(bool typedef_allowed = true) const;

      int emit_def(std::ostream&out, perm_string name) const;
      int emit_typedef(std::ostream&out, typedef_context_t&ctx) const;

      bool can_be_packed() const { return etype_->can_be_packed(); }

      bool is_unbounded() const;

      bool is_variable_length(ScopeBase*scope) const;

	// To handle subtypes
      inline void set_parent_type(const VTypeArray*parent) { parent_ = parent; }

      const VTypeArray*get_parent_type() const { return parent_; }

	// Wherever it is possible, replaces range lsb & msb expressions with
	// constant integers.
      void evaluate_ranges(ScopeBase*scope);

    private:
      int emit_with_dims_(std::ostream&out, bool packed, perm_string name) const;

	// Handles a few special types of array (*_vector, string types).
      bool write_special_case(std::ostream&out) const;
      void write_range_to_stream_(std::ostream&fd) const;

      const VType*etype_;
      std::vector<range_t> ranges_;
      bool signed_flag_;
      const VTypeArray*parent_;
};

class VTypeRange : public VType {

    public:
      VTypeRange(const VType*base);
      virtual ~VTypeRange() = 0;

      bool write_std_types(std::ostream&fd) const;
      int emit_def(std::ostream&out, perm_string name) const;
      bool type_match(const VType*that) const;

	// Get the type that is limited by the range.
      inline const VType*base_type() const { return base_; }

    protected:
      const VType*base_;
};

class VTypeRangeConst : public VTypeRange {

    public:
      VTypeRangeConst(const VType*base, int64_t end, int64_t start);

      VType*clone() const {
          return new VTypeRangeConst(base_type()->clone(), start_, end_);
      }

      int64_t start() const { return start_; }
      int64_t end() const { return end_; }

      void write_to_stream(std::ostream&fd) const;

    private:
      const int64_t start_, end_;
};

class VTypeRangeExpr : public VTypeRange {

    public:
      VTypeRangeExpr(const VType*base, Expression*end, Expression*start, bool downto);
      ~VTypeRangeExpr();

      VType*clone() const;
      int elaborate(Entity*end, ScopeBase*scope) const;

    public: // Virtual methods
      void write_to_stream(std::ostream&fd) const;

    private:
      // Boundaries
      Expression*start_, *end_;

      // Range direction (downto/to)
      bool downto_;
};

class VTypeEnum : public VType {

    public:
      explicit VTypeEnum(const std::list<perm_string>*names);
      ~VTypeEnum();

      VType*clone() const { return new VTypeEnum(*this); }

      void write_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;
      int get_width(ScopeBase*) const { return 32; }

      int emit_def(std::ostream&out, perm_string name) const;
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

	// Checks if the name is stored in the enum.
      bool has_name(perm_string name) const;

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

      VType*clone() const { return new VTypeRecord(*this); }

      void write_to_stream(std::ostream&fd) const;
      void show(std::ostream&) const;
      int get_width(ScopeBase*scope) const;
      int emit_def(std::ostream&out, perm_string name) const;

      bool can_be_packed() const { return true; }
      const element_t* element_by_name(perm_string name, int*index = NULL) const;
      inline const std::vector<element_t*> get_elements() const { return elements_; }

    private:
      std::vector<element_t*> elements_;
};

class VTypeDef : public VType {

    public:
      explicit VTypeDef(perm_string name);
      explicit VTypeDef(perm_string name, const VType*is);
      virtual ~VTypeDef();

      VType*clone() const { return new VTypeDef(*this); }

      bool type_match(const VType*that) const;

      inline perm_string peek_name() const { return name_; }

	// If the type is not given a definition in the constructor,
	// then this must be used to set the definition later.
      void set_definition(const VType*is);

	// In some situations, we only need the definition of the
	// type, and this method gets it for us.
      inline const VType* peek_definition(void) const { return type_; }

      virtual void write_to_stream(std::ostream&fd) const;
      void write_type_to_stream(std::ostream&fd) const;
      int get_width(ScopeBase*scope) const { return type_->get_width(scope); }
      int emit_typedef(std::ostream&out, typedef_context_t&ctx) const;

      int emit_def(std::ostream&out, perm_string name) const;
      int emit_decl(std::ostream&out, perm_string name, bool reg_flag) const;

      bool can_be_packed() const { return type_->can_be_packed(); }

      bool is_unbounded() const { return type_->is_unbounded(); }

    protected:
      perm_string name_;
      const VType*type_;
};

class VSubTypeDef : public VTypeDef {
    public:
      explicit VSubTypeDef(perm_string name) : VTypeDef(name) {}
      explicit VSubTypeDef(perm_string name, const VType*is) : VTypeDef(name, is) {}
      void write_typedef_to_stream(std::ostream&fd, perm_string name) const;
};

#endif /* IVL_vtype_H */
