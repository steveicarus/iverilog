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

# include  "class_type.h"
# include  "compile.h"
# include  "vpi_priv.h"
# include  <cassert>

using namespace std;

class_type::class_type(const string&nam, size_t nprop)
: class_name_(nam), properties_(nprop)
{
}

void class_type::set_property(size_t idx, const string&name)
{
      assert(idx < properties_.size());
      properties_[idx].name = name;
}

int class_type::get_type_code(void) const
{
      return vpiClassDefn;
}

static class_type*compile_class = 0;

void compile_class_start(char*lab, char*nam, unsigned ntype)
{
      assert(compile_class == 0);
      compile_class = new class_type(nam, ntype);
      compile_vpi_symbol(lab, compile_class);
      free(lab);
      delete[]nam;
}

void compile_class_property(unsigned idx, char*nam, char*typ)
{
      assert(compile_class);
      compile_class->set_property(idx, nam);
}

void compile_class_done(void)
{
      struct __vpiScope*scope = vpip_peek_current_scope();
      assert(scope);
      scope->classes[compile_class->class_name()] = compile_class;
      compile_class = 0;
}
