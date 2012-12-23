/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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


# include  "compile.h"
# include  "vpi_priv.h"
# include  "vvp_net_sig.h"
# include  "vvp_darray.h"
# include  "schedule.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>
# include  "ivl_alloc.h"

using namespace std;

__vpiDarrayVar::__vpiDarrayVar(__vpiScope*sc, const char*na, vvp_net_t*ne)
: __vpiBaseVar(sc, na, ne)
{
}

int __vpiDarrayVar::get_type_code(void) const
{ return vpiArrayVar; }


int __vpiDarrayVar::vpi_get(int code)
{
      vvp_fun_signal_object*fun = dynamic_cast<vvp_fun_signal_object*> (get_net()->fun);
      assert(fun);
      vvp_object_t val = fun->get_object();
      vvp_darray*aval = val.peek<vvp_darray>();

      switch (code) {
	  case vpiSize:
	    if (aval == 0)
		  return 0;
	    else
		  return aval->get_size();

	  default:
	    return 0;
      }
}

void __vpiDarrayVar::vpi_get_value(p_vpi_value val)
{
      val->format = vpiSuppressVal;
}

vpiHandle vpip_make_darray_var(const char*name, vvp_net_t*net)
{
      struct __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : 0;

      class __vpiDarrayVar*obj = new __vpiDarrayVar(scope, use_name, net);

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void darray_delete(vpiHandle item)
{
      class __vpiDarrayVar*obj = dynamic_cast<__vpiDarrayVar*>(item);
      delete obj;
}
#endif
