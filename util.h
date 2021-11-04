#ifndef IVL_util_H
#define IVL_util_H
/*
 * Copyright (c) 2000-2021 Stephen Williams (steve@icarus.com)
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

# include  <map>
# include  <vector>
# include  <list>
# include  "StringHeap.h"
# include  "verinum.h"

class PExpr;
class Design;
class NetScope;

/*
 * This file attempts to locate a module in a file. It operates by
 * looking for a plausible Verilog file to hold the module, and
 * invoking the parser to bring in that file's contents.
 */
extern bool load_module(const char*type, int&parser_errors);



struct attrib_list_t {
      perm_string key;
      verinum val;
};

extern attrib_list_t* evaluate_attributes(const std::map<perm_string,PExpr*>&att,
					  unsigned&natt,
					  Design*des, NetScope*scope);

#endif /* IVL_util_H */
