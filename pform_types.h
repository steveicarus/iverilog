#ifndef IVL_pform_types_H
#define IVL_pform_types_H
/*
 * Copyright (c) 2007-2021 Stephen Williams (steve@icarus.com)
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
# include  "PNamedItem.h"
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
class PScope;
class PPackage;
class PWire;
class Statement;
class netclass_t;
class netenum_t;
typedef named<PExpr*> named_pexpr_t;

/*
 * The pform_ident_t holds the identifier name and its lexical position
 * (the lexical_pos supplied by the scanner).
 */
typedef std::pair<perm_string, unsigned> pform_ident_t;

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

/* The lgate is gate instantiation information. */
struct lgate : public LineInfo {
      explicit lgate() : parms(0), parms_by_name(0), ranges(0) { }

      std::string name;
      std::list<PExpr*>*parms;
      std::list<named_pexpr_t>*parms_by_name;

      std::list<pform_range_t>*ranges;
};

/*
 * The pform_port_t holds the name and optional unpacked dimensions
 * and initialization expression for a single port in a list of port
 * declarations.
 */
struct pform_port_t {
      pform_port_t(pform_ident_t n, std::list<pform_range_t>*ud, PExpr*e)
	: name(n), udims(ud), expr(e) { }
      ~pform_port_t() { }

      pform_ident_t name;
      std::list<pform_range_t>*udims;
      PExpr*expr;
};

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
      inline name_component_t() { }
      inline explicit name_component_t(perm_string n) : name(n) { }
      ~name_component_t() { }

      // Return true if this component is nil.
      inline bool empty() const { return name.nil(); }

      perm_string name;
      std::list<index_component_t>index;
};

struct decl_assignment_t {
      pform_ident_t name;
      std::list<pform_range_t>index;
      std::unique_ptr<PExpr> expr;
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
class data_type_t : public PNamedItem {
    public:
      inline explicit data_type_t() { }
      virtual ~data_type_t() = 0;
      // This method is used by the pform dumper to diagnostic dump. The
      //  pform_dump dumps type type in pform format, and the debug_dump
      // prints the output in a linear form.
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      virtual std::ostream& debug_dump(std::ostream&out) const;

      ivl_type_t elaborate_type(Design*des, NetScope*scope);

      virtual SymbolType symbol_type() const;

    private:
	// Elaborate the type to an ivl_type_s type.
      virtual ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;
      virtual NetScope *find_scope(Design* des, NetScope *scope) const;

      bool elaborating = false;

	// Keep per-scope elaboration results cached.
      std::map<Definitions*,ivl_type_t> cache_type_elaborate_;
};

struct typedef_t : public PNamedItem {
      explicit typedef_t(perm_string n) : basic_type(ANY), name(n) { };

      ivl_type_t elaborate_type(Design*des, NetScope*scope);

      enum basic_type {
	    ANY,
	    ENUM,
	    STRUCT,
	    UNION,
	    CLASS
      };

      bool set_data_type(data_type_t *t);
      const data_type_t *get_data_type() const { return data_type.get(); }

      bool set_basic_type(basic_type bt);
      enum basic_type get_basic_type() const { return basic_type; }

protected:
      enum basic_type basic_type;
      std::unique_ptr<data_type_t> data_type;
public:
      perm_string name;
};

struct typeref_t : public data_type_t {
      explicit typeref_t(typedef_t *t, PScope *s = 0) : scope(s), type(t) {}

      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;
      NetScope *find_scope(Design* des, NetScope *scope) const;

      std::ostream& debug_dump(std::ostream&out) const;

private:
      PScope *scope;
      typedef_t *type;
};

struct type_parameter_t : data_type_t {
      explicit type_parameter_t(perm_string n) : name(n) { }
      ivl_type_t elaborate_type_raw(Design *des, NetScope *scope) const;

      perm_string name;
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
      explicit enum_type_t(data_type_t *btype) : base_type(btype) { }

	// Return the elaborated version of the type.
      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;

      SymbolType symbol_type() const;

      std::unique_ptr<data_type_t> base_type;
      std::unique_ptr< std::list<named_pexpr_t> > names;
};

struct struct_member_t : public LineInfo {
      std::unique_ptr<data_type_t> type;
      std::unique_ptr< std::list<decl_assignment_t*> > names;
      void pform_dump(std::ostream&out, unsigned indent) const;
};

struct struct_type_t : public data_type_t {
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;

      bool packed_flag;
      bool union_flag;
      bool signed_flag;
      std::unique_ptr< std::list<struct_member_t*> > members;
};

struct atom_type_t : public data_type_t {
      enum type_code {
	    INTEGER,
	    TIME,
	    BYTE,
	    SHORTINT,
	    INT,
	    LONGINT
      };

      explicit atom_type_t(enum type_code tc, bool flag) : type_code(tc),
							   signed_flag(flag) { }

      enum type_code type_code;
      bool signed_flag;

      virtual std::ostream& debug_dump(std::ostream&out) const;

      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;
};

extern atom_type_t size_type;

/*
 * The vector_type_t class represents types in the old Verilog
 * way. Some typical examples:
 *
 *   logic signed [7:0] foo
 *   bit unsigned foo
 *   reg foo
 *
 * There is one special case:
 *
 * If there are no reg/logic/bit/bool keywords, then Verilog will
 * assume the type is logic, but the context may need to know about
 * this case, so the implicit_flag member is set to true in that case.
 */
struct vector_type_t : public data_type_t {
      inline explicit vector_type_t(ivl_variable_type_t bt, bool sf,
				    std::list<pform_range_t>*pd)
      : base_type(bt), signed_flag(sf), integer_flag(false), implicit_flag(false), pdims(pd) { }
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      virtual std::ostream& debug_dump(std::ostream&out) const;
      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;

      ivl_variable_type_t base_type;
      bool signed_flag;
      bool integer_flag; // True if "integer" was used
      bool implicit_flag; // True if this type is implicitly logic/reg
      std::unique_ptr< std::list<pform_range_t> > pdims;
};

struct array_base_t : public data_type_t {
    public:
      inline explicit array_base_t(data_type_t*btype, std::list<pform_range_t>*pd)
      : base_type(btype), dims(pd) { }

      std::unique_ptr<data_type_t> base_type;
      std::unique_ptr< std::list<pform_range_t> > dims;
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

      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;
};

/*
 * The uarray_type_t represents unpacked array types.
 */
struct uarray_type_t : public array_base_t {
      inline explicit uarray_type_t(data_type_t*btype, std::list<pform_range_t>*pd)
      : array_base_t(btype, pd) { }

    public:
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;
};

struct real_type_t : public data_type_t {
 public:
      enum type_t { REAL, SHORTREAL };
      inline explicit real_type_t(type_t tc) : type_code_(tc) { }
      virtual std::ostream& debug_dump(std::ostream&out) const;

      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;

      inline type_t type_code() const { return type_code_; }

 private:
      type_t type_code_;
};

struct string_type_t : public data_type_t {
      inline explicit string_type_t() { }
      ~string_type_t();

      ivl_type_t elaborate_type_raw(Design*des, NetScope*scope) const;
};

struct class_type_t : public data_type_t {

      inline explicit class_type_t(perm_string n) : name(n) { }

      void pform_dump(std::ostream&out, unsigned indent) const;
      void pform_dump_init(std::ostream&out, unsigned indent) const;

	// This is the named type that is supposed to be the base
	// class that we are extending. This is nil if there is no
	// hierarchy. If there are arguments to the base class, then
	// put them in the base_args vector.
      std::unique_ptr<data_type_t> base_type;
      std::vector<named_pexpr_t> base_args;

      bool virtual_class;

	// This is a map of the properties. Map the name to the type.
      struct prop_info_t : public LineInfo {
	    inline prop_info_t() : qual(property_qualifier_t::make_none()) { }
	    inline prop_info_t(property_qualifier_t q, data_type_t*t) : qual(q), type(t) { }
	    prop_info_t(prop_info_t&&) = default;
	    prop_info_t& operator=(prop_info_t&&) = default;
	    property_qualifier_t qual;
	    std::unique_ptr<data_type_t> type;
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

      ivl_type_t elaborate_type_raw(Design*, NetScope*) const;

      perm_string name;

      virtual SymbolType symbol_type() const;
};

ivl_type_t elaborate_array_type(Design *des, NetScope *scope,
			        const LineInfo &li, ivl_type_t base_type,
			        const std::list<pform_range_t> &dims);

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

struct pform_scoped_name_t {
      pform_scoped_name_t() = default;
      pform_scoped_name_t(PPackage *p, const pform_name_t &n) : package(p),
							        name(n) {}
      pform_scoped_name_t(const pform_name_t &n) : name(n) {}

      const name_component_t& back() const { return name.back(); }
      size_t size() const { return name.size(); }

      PPackage *package = nullptr;
      pform_name_t name;
};

inline perm_string peek_head_name(const pform_name_t&that)
{
      return that.front().name;
}

inline perm_string peek_tail_name(const pform_name_t&that)
{
      return that.back().name;
}

inline perm_string peek_head_name(const pform_scoped_name_t &that)
{
      return peek_head_name(that.name);
}

inline perm_string peek_tail_name(const pform_scoped_name_t &that)
{
      return peek_tail_name(that.name);
}

/*
 * In pform names, the "super" and "this" keywords are converted to
 * These tokens so that they don't interfere with the namespace and
 * are handled specially.
 */
# define SUPER_TOKEN "#"
# define THIS_TOKEN  "@"

static inline std::ostream& operator<< (std::ostream&out, const data_type_t&that)
{
      return that.debug_dump(out);
}

extern std::ostream& operator<< (std::ostream&out, const pform_name_t&);
extern std::ostream& operator<< (std::ostream&out, const pform_scoped_name_t&);
extern std::ostream& operator<< (std::ostream&out, const name_component_t&that);
extern std::ostream& operator<< (std::ostream&out, const index_component_t&that);
extern std::ostream& operator<< (std::ostream&out, enum typedef_t::basic_type bt);

#endif /* IVL_pform_types_H */
