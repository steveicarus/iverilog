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

# include  "pform.h"
# include  "PClass.h"

static PClass*pform_cur_class = 0;

void pform_start_class_declaration(const struct vlltype&loc, class_type_t*type)
{
      PClass*class_scope = pform_push_class_scope(loc, type->name);
      assert(pform_cur_class == 0);
      pform_cur_class = class_scope;
}

void pform_end_class_declaration(void)
{
      assert(pform_cur_class);
      pform_cur_class = 0;
      pform_pop_scope();
}
