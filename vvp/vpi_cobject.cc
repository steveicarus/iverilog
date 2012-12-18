/*
 * Copyright (c) 2012 Picture Elements, Inc.
 *    Stephen Williams (steve@icarus.com)
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
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif

__vpiCobjectVar::__vpiCobjectVar(__vpiScope*sc, const char*na, vvp_net_t*ne)
: __vpiBaseVar(sc, na, ne)
{
}

int __vpiCobjectVar::get_type_code(void) const
{ return vpiClassVar; }

int __vpiCobjectVar::vpi_get(int)
{
      return 0;
}

void __vpiCobjectVar::vpi_get_value(p_vpi_value val)
{
      val->format = vpiSuppressVal;
}

vpiHandle vpip_make_cobject_var(const char*name, vvp_net_t*net)
{
      struct __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : 0;

      class __vpiCobjectVar*obj = new __vpiCobjectVar(scope, use_name, net);

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void class_delete(vpiHandle item)
{
      class __vpiCobjectVar*obj = dynamic_cast<__vpiCobjectVar*>(item);
      delete obj;
}
#endif
