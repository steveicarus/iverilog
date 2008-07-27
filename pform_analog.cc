/*
 * Copyright (c) 2008 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "compiler.h"
# include  "pform.h"
# include  "parse_misc.h"
# include  "AStatement.h"

AStatement* pform_contribution_statement(const struct vlltype&loc)
{
      AContrib*tmp = new AContrib;
      FILE_NAME(tmp, loc);
      return tmp;
}

void pform_make_analog_behavior(const struct vlltype&loc, AProcess::Type pt,
				AStatement*statement)
{
      AProcess*proc = new AProcess(pt, statement);
      FILE_NAME(proc, loc);

      pform_put_behavior_in_scope(proc);
}
