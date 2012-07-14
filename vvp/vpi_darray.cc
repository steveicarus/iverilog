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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
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

class __vpiDarrayVar : public __vpiHandle {

    public:
      __vpiDarrayVar(__vpiScope*scope, const char*name, vvp_net_t*net);

      int get_type_code(void) const;
      int vpi_get(int code);
      void vpi_get_value(p_vpi_value val);

      inline vvp_net_t* get_net() const { return net_; }

    private:
      struct __vpiScope* scope_;
      const char*name_;
      vvp_net_t*net_;
};

__vpiDarrayVar::__vpiDarrayVar(__vpiScope*sc, const char*na, vvp_net_t*ne)
: scope_(sc), name_(na), net_(ne)
{
}

int __vpiDarrayVar::get_type_code(void) const
{ return vpiArrayVar; }


int __vpiDarrayVar::vpi_get(int code)
{
      vvp_fun_signal_object*fun = dynamic_cast<vvp_fun_signal_object*> (net_->fun);
      assert(fun);
      vvp_object_t val = fun->get_object();

      switch (code) {
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

      struct __vpiDarrayVar*obj = new __vpiDarrayVar(scope, use_name, net);

      return obj;
}
