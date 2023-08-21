/*
 * Copyright (c) 2006-2011 Stephen Williams <steve@icarus.com>
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

# include  "PSpec.h"

PSpecPath::PSpecPath(const std::list<perm_string> &src_list,
		     const std::list<perm_string> &dst_list,
		     char polarity, bool full_flag)
: conditional(false), condition(0), edge(0),
  src(src_list.begin(), src_list.end()), dst(dst_list.begin(), dst_list.end()),
  data_source_expression(0)
{
      full_flag_ = full_flag;
      polarity_ = polarity;
}

PSpecPath::~PSpecPath()
{
}
