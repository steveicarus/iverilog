/*
 * Copyright (c) 2026 Icarus UVM track
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 */

# include  "netaarray.h"
# include  <iostream>

using namespace std;

netaarray_t::netaarray_t(ivl_type_t vec)
: netarray_t(vec)
{
}

netaarray_t::~netaarray_t()
{
}

ivl_variable_type_t netaarray_t::base_type(void) const
{
      return IVL_VT_AARRAY;
}

bool netaarray_t::test_equivalence(ivl_type_t that) const
{
      const netaarray_t* that_aa = dynamic_cast<const netaarray_t*>(that);
      if (!that_aa) return false;
      return test_compatibility(that);
}

bool netaarray_t::test_compatibility(ivl_type_t that) const
{
      const netaarray_t *that_aa = dynamic_cast<const netaarray_t*>(that);
      if (!that_aa)
	    return false;

      return element_type()->type_equivalent(that_aa->element_type());
}

ostream& netaarray_t::debug_dump(ostream&out) const
{
      out << "aarray<" << *element_type() << ">[string]";
      return out;
}
