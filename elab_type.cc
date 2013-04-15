/*
 * Copyright (c) 2012-2013 Stephen Williams (steve@icarus.com)
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
# include  "netscalar.h"
# include  "netvector.h"
# include  "netmisc.h"
# include  <typeinfo>
# include  "ivl_assert.h"

using namespace std;

ivl_type_s* data_type_t::elaborate_type(Design*des, NetScope*) const
{
      cerr << get_fileline() << ": internal error: "
	   << "Elaborate method not implemented for " << typeid(*this).name()
	   << "." << endl;
      des->errors += 1;
      return 0;
}

ivl_type_s* atom2_type_t::elaborate_type(Design*des, NetScope*) const
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

ivl_type_s* class_type_t::elaborate_type(Design*, NetScope*scope) const
{
      return scope->find_class(name);
}

ivl_type_s* vector_type_t::elaborate_type(Design*des, NetScope*scope) const
{
      vector<netrange_t> packed;

      if (pdims.get()) {
	    for (list<pform_range_t>::const_iterator cur = pdims->begin()
		       ; cur != pdims->end() ; ++ cur) {

		  NetExpr*me = elab_and_eval(des, scope, cur->first, 0, true);
		  assert(me);

		  NetExpr*le = elab_and_eval(des, scope, cur->second, 0, true);
		  assert(le);

		  long mnum = 0, lnum = 0;
		  if ( ! eval_as_long(mnum, me) ) {
			assert(0);
			des->errors += 1;
		  }

		  if ( ! eval_as_long(lnum, le) ) {
			assert(0);
			des->errors += 1;
		  }

		  packed.push_back(netrange_t(mnum, lnum));
	    }
      }

      netvector_t*tmp = new netvector_t(packed, base_type);
      tmp->set_signed(signed_flag);
      tmp->set_isint(integer_flag);

      return tmp;
}

ivl_type_s* real_type_t::elaborate_type(Design*, NetScope*) const
{
      switch (type_code) {
	  case REAL:
	    return &netreal_t::type_real;
	  case SHORTREAL:
	    return &netreal_t::type_shortreal;
      }
      return 0;
}

ivl_type_s* string_type_t::elaborate_type(Design*, NetScope*) const
{
      return &netstring_t::type_string;
}

ivl_type_s* uarray_type_t::elaborate_type(Design*des, NetScope*scope) const
{

      ivl_type_t btype = base_type->elaborate_type(des, scope);

      assert(dims->size() == 1);
      list<pform_range_t>::const_iterator cur = dims->begin();
      assert(cur->first == 0 && cur->second==0);
      ivl_type_s*res = new netdarray_t(btype);
      return res;
}
