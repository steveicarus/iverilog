#ifndef IVL_ivl_assert_H
#define IVL_ivl_assert_H
/*
 * Copyright (c) 2007-2021 Stephen Williams (steve@icarus.com)
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

# include  <cstdlib>

#define ivl_assert(tok, expression)  \
      do { \
	    if (! (expression)) { \
		  std::cerr << (tok).get_fileline() << ": assert: " \
			    << __FILE__ << ":" << __LINE__ \
			    << ": failed assertion " << #expression << std::endl; \
		  abort(); \
	    } \
      } while (0)

#endif /* IVL_ivl_assert_H */
