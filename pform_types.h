#ifndef __pform_types_H
#define __pform_types_H
/*
 * Copyright (c) 2007 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: pform_types.h,v 1.2 2007/06/04 02:19:07 steve Exp $"
#endif

// This for the perm_string type.
# include  "StringHeap.h"
# include  <iostream>
# include  <list>

/*
 * parse-form types.
 */

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

extern bool operator < (const name_component_t&lef, const name_component_t&rig);

/*
 * The pform_name_t is the general form for a hierarchical identifier.
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
