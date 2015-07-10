/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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

# include  "pform_types.h"
# include  "netlist.h"
# include  "netclass.h"
# include  "netdarray.h"
# include  "netenum.h"
# include  "netparray.h"
# include  "netscalar.h"
# include  "netstruct.h"
# include  "netvector.h"
# include  "netmisc.h"
# include  <typeinfo>
# include  "ivl_assert.h"

using namespace std;

/*
 * Some types have a list of ranges that need to be elaborated. This
 * function elaborates the ranges referenced by "dims" into the vector
 * "ranges".
 */
static void elaborate_array_ranges(Design*des, NetScope*scope,
				   vector<netrange_t>&ranges,
				   const list<pform_range_t>*dims)
{
      if (dims == 0)
	    return;

      for (list<pform_range_t>::const_iterator cur = dims->begin()
		 ; cur != dims->end() ; ++ cur) {

	    NetExpr*me = elab_and_eval(des, scope, cur->first, 0, true);

	    NetExpr*le = elab_and_eval(des, scope, cur->second, 0, true);

	      /* If elaboration failed for either expression, we
		 should have already reported the error, so just
		 skip the following evaluation to recover. */

	    long mnum = 0, lnum = 0;
	    if ( me && ! eval_as_long(mnum, me) ) {
		  assert(0);
		  des->errors += 1;
	    }

	    if ( le && ! eval_as_long(lnum, le) ) {
		  assert(0);
		  des->errors += 1;
	    }

	    ranges.push_back(netrange_t(mnum, lnum));
      }
}

/*
 * Elaborations of types may vary depending on the scope that it is
 * done in, so keep a per-scope cache of the results.
 */
ivl_type_s* data_type_t::elaborate_type(Design*des, NetScope*scope)
{
      Definitions*use_definitions = scope;
      if (use_definitions == 0)
	    use_definitions = des;

      map<Definitions*,ivl_type_s*>::iterator pos = cache_type_elaborate_.lower_bound(use_definitions);
	  if (pos != cache_type_elaborate_.end() && pos->first == use_definitions)
	     return pos->second;

      ivl_type_s*tmp = elaborate_type_raw(des, scope);
      cache_type_elaborate_.insert(pos, pair<NetScope*,ivl_type_s*>(scope, tmp));
      return tmp;
}

ivl_type_s* data_type_t::elaborate_type_raw(Design*des, NetScope*) const
{
      cerr << get_fileline() << ": internal error: "
	   << "Elaborate method not implemented for " << typeid(*this).name()
	   << "." << endl;
      des->errors += 1;
      return 0;
}

ivl_type_s* atom2_type_t::elaborate_type_raw(Design*des, NetScope*) const
{
      switch (type_code) {
	  case 64:
	    if (signed_flag)
		  return &netvector_t::atom2s64;
	    else
		  return &netvector_t::atom2u64;

	  case 32:
	    if (signed_flag)
		  return &netvector_t::atom2s32;
	    else
		  return &netvector_t::atom2u32;

	  case 16:
	    if (signed_flag)
		  return &netvector_t::atom2s16;
	    else
		  return &netvector_t::atom2u16;

	  case 8:
	    if (signed_flag)
		  return &netvector_t::atom2s8;
	    else
		  return &netvector_t::atom2u8;

	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "atom2_type_t type_code=" << type_code << "." << endl;
	    des->errors += 1;
	    return 0;
      }
}

ivl_type_s* class_type_t::elaborate_type_raw(Design*, NetScope*) const
{
      ivl_assert(*this, save_elaborated_type);
      return save_elaborated_type;
}

/*
 * elaborate_type_raw for enumerations is actually mostly performed
 * during scope elaboration so that the enumeration literals are
 * available at the right time. At that time, the netenum_t* object is
 * stashed in the scope so that I can retrieve it here.
 */
ivl_type_s* enum_type_t::elaborate_type_raw(Design*des, NetScope*scope) const
{
      ivl_assert(*this, scope);
      ivl_type_s*tmp = scope->enumeration_for_key(this);
      if (tmp) return tmp;

      tmp = des->enumeration_for_key(this);
      return tmp;
}

ivl_type_s* vector_type_t::elaborate_type_raw(Design*des, NetScope*scope) const
{
      vector<netrange_t> packed;
      elaborate_array_ranges(des, scope, packed, pdims.get());

      netvector_t*tmp = new netvector_t(packed, base_type);
      tmp->set_signed(signed_flag);
      tmp->set_isint(integer_flag);

      return tmp;
}

ivl_type_s* real_type_t::elaborate_type_raw(Design*, NetScope*) const
{
      switch (type_code) {
	  case REAL:
	    return &netreal_t::type_real;
	  case SHORTREAL:
	    return &netreal_t::type_shortreal;
      }
      return 0;
}

ivl_type_s* string_type_t::elaborate_type_raw(Design*, NetScope*) const
{
      return &netstring_t::type_string;
}

ivl_type_s* parray_type_t::elaborate_type_raw(Design*des, NetScope*scope) const
{
      vector<netrange_t>packed;
      elaborate_array_ranges(des, scope, packed, dims.get());

      ivl_type_t etype = base_type->elaborate_type(des, scope);

      return new netparray_t(packed, etype);
}

netstruct_t* struct_type_t::elaborate_type_raw(Design*des, NetScope*scope) const
{
      netstruct_t*res = new netstruct_t;

      res->packed(packed_flag);

      if (union_flag)
	    res->union_flag(true);

      for (list<struct_member_t*>::iterator cur = members->begin()
		 ; cur != members->end() ; ++ cur) {

	      // Elaborate the type of the member.
	    struct_member_t*curp = *cur;
	    ivl_type_t mem_vec = curp->type->elaborate_type(des, scope);
	    if (mem_vec == 0)
		  continue;

	      // There may be several names that are the same type:
	      //   <data_type> name1, name2, ...;
	      // Process all the member, and give them a type.
	    for (list<decl_assignment_t*>::iterator name = curp->names->begin()
		       ; name != curp->names->end() ;  ++ name) {
		  decl_assignment_t*namep = *name;

		  netstruct_t::member_t memb;
		  memb.name = namep->name;
		  memb.net_type = mem_vec;
		  res->append_member(des, memb);
	    }
      }

      return res;
}

ivl_type_s* uarray_type_t::elaborate_type_raw(Design*des, NetScope*scope) const
{

      ivl_type_t btype = base_type->elaborate_type(des, scope);

      assert(dims->size() >= 1);
      list<pform_range_t>::const_iterator cur = dims->begin();

	// Special case: if the dimension is nil:nil, this is a
	// dynamic array. Note that we only know how to handle dynamic
	// arrays with 1 dimension at a time.
      if (cur->first==0 && cur->second==0) {
	    assert(dims->size()==1);
	    ivl_type_s*res = new netdarray_t(btype);
	    return res;
      }

      vector<netrange_t> dimensions;
      bool bad_range = evaluate_ranges(des, scope, dimensions, *dims);

      if (bad_range) {
	    cerr << get_fileline() << " : warning: "
		 << "Bad dimensions for type here." << endl;
      }

      ivl_assert(*this, btype);
      ivl_type_s*res = new netuarray_t(dimensions, btype);
      return res;
}
