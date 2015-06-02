#ifndef IVL_pform_types_H
#define IVL_pform_types_H
/*
 * Copyright (c) 2007-2014 Stephen Williams (steve@icarus.com)
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

// This for the perm_string type.
# include  "StringHeap.h"
# include  "LineInfo.h"
# include  "verinum.h"
# include  "named.h"
# include  "netstruct.h"
# include  "property_qual.h"
# include  "ivl_target.h"
# include  <iostream>
# include  <list>
# include  <vector>
# include  <map>
# include  <memory>

/*
 * parse-form types.
 */

class Design;
class NetScope;
class Definitions;
class PExpr;
class PWire;
class Statement;
class ivl_type_s;
class netclass_t;
class netenum_t;
typedef named<verinum> named_number_t;
typedef named<PExpr*> named_pexpr_t;

/*
 * The pform_range_t holds variable dimensions for type
 * declarations. The two expressions are interpreted as the first and
 * last values of the range. For example:
 *
 *   [<expr1> : <expr2>]  -- Normal array range
 *       first == <expr1>
 *       second = <expr2>
 *
 *   [<expr>] -- SystemVerilog canonical range
 *       first = PENumber(0)
 *       second = <expr> - 1;
 *
 *   [ ] -- Dynamic array
 *       first = 0
 *       second = 0
 *
 *   [ $ ] -- Queue type
 *       first = PENull
 *       second = 0
 */
typedef std::pair<PExpr*,PExpr*> pform_range_t;

/*
 * Semantic NOTES:
 * - The SEL_BIT is a single expression. This might me a bit select
 * of a vector, or a word select of an array.
 *
 * - The SEL_BIT_LAST index component is an array/queue [$] index,
 * that is the last item in the variable.
 */
struct index_component_t {
      enum ctype_t { SEL_NONE, SEL_BIT, SEL_BIT_LAST, SEL_PART, SEL_IDX_UP, SEL_IDX_DO };

      index_component_t() : sel(SEL_NONE), msb(0), lsb(0) { };
      ~index_component_t() { }

      ctype_t sel;
      class PExpr*msb;
      class PExpr*lsb;
};

struct name_component_t {
      explicit name_component_t(perm_string n) : name(n) { }
      ~name_component_t() { }

      perm_string name;
      std::list<index_component_t>index;
};

struct decl_assignment_t {
      perm_string name;
      std::list<pform_range_t>index;
      std::auto_ptr<PExpr> expr;
};

struct pform_tf_port_t {
      PWire*port;
      PExpr*defe;

      inline pform_tf_port_t() : port(0), defe(0) { }
      inline explicit pform_tf_port_t(PWire*p) : port(p), defe(0) { }
};

/*
 * This is the base class for data types that are matched by the
 * "data_type" rule in the parse rule. We make the type virtual so
 * that dynamic types will work.
 */
class data_type_t : public LineInfo {
    public:
      inline explicit data_type_t() { }
      virtual ~data_type_t() = 0;
	// This method is used to figure out the base type of a packed
	// compound object. Return IVL_VT_NO_TYPE if the type is not packed.
      virtual ivl_variable_type_t figure_packed_base_type(void)const;
	// This method is used by the pform dumper to diagnostic dump.
      virtual void pform_dump(std::ostream&out, unsigned indent) const;

      ivl_type_s* elaborate_type(Design*des, NetScope*scope);

    private:
	// Elaborate the type to an ivl_type_s type.
      virtual ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;

	// Keep per-scope elaboration results cached.
      std::map<Definitions*,ivl_type_s*> cache_type_elaborate_;
};

struct void_type_t : public data_type_t {
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
};

/*
 * The enum_type_t holds the parsed declaration to represent an
 * enumeration. Since this is in the pform, it represents the type
 * before elaboration so the range, for example, may not be complete
 * until it is elaborated in a scope.
 */
struct enum_type_t : public data_type_t {
	// Return the elaborated version of the type.
      virtual ivl_type_s*elaborate_type_raw(Design*des, NetScope*scope) const;

      ivl_variable_type_t base_type;
      bool signed_flag;
      bool integer_flag; // True if "integer" was used
      std::auto_ptr< list<pform_range_t> > range;
      std::auto_ptr< list<named_pexpr_t> > names;
      LineInfo li;
};

struct struct_member_t : public LineInfo {
      std::auto_ptr<data_type_t> type;
      std::auto_ptr< list<decl_assignment_t*> > names;
      void pform_dump(std::ostream&out, unsigned indent) const;
};

struct struct_type_t : public data_type_t {
      virtual ivl_variable_type_t figure_packed_base_type(void)const;
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      virtual netstruct_t* elaborate_type_raw(Design*des, NetScope*scope) const;

      bool packed_flag;
      bool union_flag;
      std::auto_ptr< list<struct_member_t*> > members;
};

struct atom2_type_t : public data_type_t {
      inline explicit atom2_type_t(int tc, bool flag)
      : type_code(tc), signed_flag(flag) { }
      int type_code;
      bool signed_flag;

      ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;
};

extern atom2_type_t size_type;

/*
 * The vector_type_t class represents types in the old Verilog
 * way. Some typical examples:
 *
 *   logic signed [7:0] foo
 *   bit unsigned foo
 *   reg foo
 *
 * There are a few special cases:
 *
 * For the most part, Verilog treats "logic" and "reg" as synonyms,
 * but there are a few cases where the parser needs to know the
 * difference. So "reg_flag" is set to true if the IVL_VT_LOGIC type
 * is due to the "reg" keyword.
 *
 * If there are no reg/logic/bit/bool keywords, then Verilog will
 * assume the type is logic, but the context may need to know about
 * this case, so the implicit_flag member is set to true in that case.
 */
struct vector_type_t : public data_type_t {
      inline explicit vector_type_t(ivl_variable_type_t bt, bool sf,
				    std::list<pform_range_t>*pd)
      : base_type(bt), signed_flag(sf), reg_flag(false), integer_flag(false), implicit_flag(false), pdims(pd) { }
      virtual ivl_variable_type_t figure_packed_base_type(void)const;
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;

      ivl_variable_type_t base_type;
      bool signed_flag;
      bool reg_flag; // True if "reg" was used
      bool integer_flag; // True if "integer" was used
      bool implicit_flag; // True if this type is implicitly logic/reg
      std::auto_ptr< list<pform_range_t> > pdims;
};

struct array_base_t : public data_type_t {
    public:
      inline explicit array_base_t(data_type_t*btype, std::list<pform_range_t>*pd)
      : base_type(btype), dims(pd) { }

      data_type_t*base_type;
      std::auto_ptr< list<pform_range_t> > dims;
};

/*
 * The parray_type_t is a generalization of the vector_type_t in that
 * the base type is another general data type. Ultimately, the subtype
 * must also be packed (as this is a packed array) but that may be
 * worked out during elaboration.
 */
struct parray_type_t : public array_base_t {
      inline explicit parray_type_t(data_type_t*btype, std::list<pform_range_t>*pd)
      : array_base_t(btype, pd) { }

      virtual ivl_variable_type_t figure_packed_base_type(void)const;
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      virtual ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;
};

/*
 * The uarray_type_t represents unpacked array types.
 */
struct uarray_type_t : public array_base_t {
      inline explicit uarray_type_t(data_type_t*btype, std::list<pform_range_t>*pd)
      : array_base_t(btype, pd) { }

    public:
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      virtual ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;
};

struct real_type_t : public data_type_t {
      enum type_t { REAL, SHORTREAL };
      inline explicit real_type_t(type_t tc) : type_code(tc) { }
      type_t type_code;

      ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;
};

struct string_type_t : public data_type_t {
      inline explicit string_type_t() { }
      ~string_type_t();

      ivl_type_s* elaborate_type_raw(Design*des, NetScope*scope) const;
};

struct class_type_t : public data_type_t {

      inline explicit class_type_t(perm_string n)
      : name(n), base_type(0), save_elaborated_type(0) { }

      void pform_dump(std::ostream&out, unsigned indent) const;
      void pform_dump_init(std::ostream&out, unsigned indent) const;

	// This is the name of the class type.
      perm_string name;

	// This is the named type that is supposed to be the base
	// class that we are extending. This is nil if there is no
	// hierarchy. If there are arguments to the base class, then
	// put them in the base_args vector.
      data_type_t*base_type;
      std::list<PExpr*>base_args;

	// This is a map of the properties. Map the name to the type.
      struct prop_info_t {
	    inline prop_info_t() : qual(property_qualifier_t::make_none()), type(0) { }
	    inline prop_info_t(property_qualifier_t q, data_type_t*t) : qual(q), type(t) { }
	    property_qualifier_t qual;
	    data_type_t* type;
      };
      std::map<perm_string, struct prop_info_t> properties;

	// This is an ordered list of property initializers. The name
	// is the name of the property to be assigned, and the val is
	// the expression that is assigned.
      std::vector<Statement*> initialize;

	// This is an ordered list of property initializers for static
	// properties. These are run in a synthetic "initial" block
	// without waiting for any constructor.
      std::vector<Statement*> initialize_static;

      ivl_type_s* elaborate_type_raw(Design*, NetScope*) const;
	// The save_elaborated_type member must be set to the pointer
	// to the netclass_t object that is created to represent this
	// type. The elaborate_type_raw() method uses this pointer,
	// and it is used in some other situations as well.
      netclass_t* save_elaborated_type;
};

/*
 * The pform_name_t is the general form for a hierarchical
 * identifier. It is an ordered list of name components. Each name
 * component is an identifier and an optional list of bit/part
 * selects. The simplest name component is a simple identifier:
 *
 *    foo
 *
 * The bit/part selects come from the source and are made part of the
 * name component. A bit select is a single number that may be a bit
 * select of a vector or a word select of an array:
 *
 *    foo[5]     -- a bit select/word index
 *    foo[6:4]   -- a part select
 *
 * The index components of a name component are collected into an
 * ordered list, so there may be many, for example:
 *
 *    foo[5][6:4] -- a part select of an array word
 *
 * The pform_name_t, then, is an ordered list of these name
 * components. The list of names comes from a hierarchical name in the
 * source, like this:
 *
 *    foo[5].bar[6:4]  -- a part select of a vector in sub-scope foo[5].
 */
typedef std::list<name_component_t> pform_name_t;


inline perm_string peek_head_name(const pform_name_t&that)
{
      return that.front().name;
}

inline perm_string peek_tail_name(const pform_name_t&that)
{
      return that.back().name;
}

extern std::ostream& operator<< (std::ostream&out, const pform_name_t&);
extern std::ostream& operator<< (std::ostream&out, const name_component_t&that);
extern std::ostream& operator<< (std::ostream&out, const index_component_t&that);

#endif /* IVL_pform_types_H */
