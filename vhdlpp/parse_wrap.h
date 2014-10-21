#ifndef IVL_parse_wrap_H
#define IVL_parse_wrap_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
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

/*
 * This header wraps the parse.h header file that is generated from
 * the parse.y source file. This is used to include definitions that
 * are needed by the parse type, etc.
 */

# include  <list>
# include "vhdlint.h"
# include "vhdlreal.h"
# include  "architec.h"
# include  "expression.h"
# include  "sequential.h"
# include  "subprogram.h"
# include  "parse_types.h"

class VType;

# include  "parse.h"

#endif /* IVL_parse_wrap_H */
