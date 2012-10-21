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

# include  "pform.h"
# include  "PPackage.h"
# include  "parse_misc.h"
# include  "ivl_assert.h"

static PPackage*pform_cur_package = 0;

void pform_start_package_declaration(const struct vlltype&loc, const char*name)
{
      VLerror(loc, "sorry: Package declarations not supported.");
      ivl_assert(loc, pform_cur_package == 0);

      perm_string use_name = lex_strings.make(name);
      PPackage*pkg_scope = pform_push_package_scope(loc, use_name);
      pform_cur_package = pkg_scope;
}

void pform_end_package_declaration(const struct vlltype&loc)
{
      ivl_assert(loc, pform_cur_package);
      pform_cur_package = 0;
      pform_pop_scope();
}
