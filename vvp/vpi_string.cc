/*
 * Copyright (c) 2012-2022 Stephen Williams (steve@icarus.com)
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

__vpiStringVar::__vpiStringVar(__vpiScope*sc, const char*na, vvp_net_t*ne)
: __vpiBaseVar(sc, na, ne)
{
}

int __vpiStringVar::get_type_code(void) const
{ return vpiStringVar; }

int __vpiStringVar::vpi_get(int code)
{
      vvp_fun_signal_string*fun = dynamic_cast<vvp_fun_signal_string*> (get_net()->fun);
      assert(fun);
      string str = fun->get_string();

      switch (code) {
	  case vpiSize:
	      // The vpiSize of a string variable is the number of
	      // bytes (characters) in that string.
	    return str.size();
	  case vpiLeftRange:
	    return 0;
	  case vpiRightRange:
            return str.size() - 1;
	  default:
	    fprintf(stderr, "vpi sorry: property is not implemented");
	    assert(false);
	    return 0;
      }
}

void __vpiStringVar::vpi_get_value(p_vpi_value val)
{
      vvp_fun_signal_string*fun = dynamic_cast<vvp_fun_signal_string*> (get_net()->fun);
      assert(fun);
      string str = fun->get_string();

      if (val->format == vpiStringVal || val->format == vpiObjTypeVal) {
	    char*rbuf = (char *) need_result_buf(str.size()+1, RBUF_VAL);
	    strcpy(rbuf, str.c_str());
	    val->format = vpiStringVal;
	    val->value.str = rbuf;
	    return;
      }

      val->format = vpiSuppressVal;
}

vpiHandle __vpiStringVar::vpi_put_value(p_vpi_value val, int)
{
      vvp_fun_signal_string*fun = dynamic_cast<vvp_fun_signal_string*> (get_net()->fun);
      assert(fun);
      assert(val->format == vpiStringVal);
      vvp_net_ptr_t dest (get_net(), 0);
      vvp_send_string(dest, val->value.str, vthread_get_wt_context());
      return 0;
}

vpiHandle vpip_make_string_var(const char*name, vvp_net_t*net)
{
      __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : NULL;

      __vpiStringVar*obj = new __vpiStringVar(scope, use_name, net);

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void string_delete(vpiHandle item)
{
      __vpiStringVar*obj = dynamic_cast<__vpiStringVar*>(item);
      delete obj;
}
#endif
