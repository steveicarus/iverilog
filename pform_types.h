#ifndef __pform_types_H
#define __pform_types_H
/*
 * Copyright (c) 2007-2011 Stephen Williams (steve@icarus.com)
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

// This for the perm_string type.
# include  "StringHeap.h"
# include  "LineInfo.h"
# include  "verinum.h"
# include  "named.h"
# include  "ivl_target.h"
# include  <iostream>
# include  <list>
# include  <map>
# include  <memory>

/*
 * parse-form types.
 */

class PExpr;
typedef named<verinum> named_number_t;
typedef named<PExpr*> named_pexpr_t;

struct index_component_t {
      enum ctype_t { SEL_NONE, SEL_BIT, SEL_PART, SEL_IDX_UP, SEL_IDX_DO };

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
      std::list<index_component_t>index;
      std::auto_ptr<PExpr> expr;
};

/*
 * This is the base class for data types that are matched by the
 * "data_type" rule in the parse rule. We make the type virtual so
 * that dynamic types will work.
 */
struct data_type_t : public LineInfo {
      virtual ~data_type_t() = 0;

	// This method is used by the pform dumper to diagnostic dump.
      virtual void pform_dump(std::ostream&out, unsigned indent) const;
};

/*
 * The enum_type_t holds the parsed declaration to represent an
 * enumeration. Since this is in the pform, it represents the type
 * before elaboration to the range, for example, man not be complete
 * until it is elaborated in a scope.
 */
struct enum_type_t : public data_type_t {
      ivl_variable_type_t base_type;
      bool signed_flag;
      std::auto_ptr< list<PExpr*> > range;
      std::auto_ptr< list<named_pexpr_t> > names;
      LineInfo li;
};

struct struct_member_t : public LineInfo {
      ivl_variable_type_t type;
      std::auto_ptr< list<PExpr*> > range;
      std::auto_ptr< list<decl_assignment_t*> > names;
};

struct struct_type_t : public data_type_t {
      bool packed_flag;
      std::auto_ptr< list<struct_member_t*> > members;
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

#endif
