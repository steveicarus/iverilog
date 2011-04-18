#ifndef __parse_misc_H
#define __parse_misc_H
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "parse_api.h"

class ActiveScope;
class Architecture;
class Expression;
class Package;
class ScopeBase;
class VType;

extern void bind_architecture_to_entity(const char*ename, Architecture*arch);

extern const VType* calculate_subtype_array(const YYLTYPE&loc, const char*base_name,
					    ScopeBase*scope,
					    Expression*array_left,
					    bool downto,
					    Expression*array_right);
extern const VType* calculate_subtype_range(const YYLTYPE&loc, const char*base_name,
					    ScopeBase*scope,
					    Expression*range_left,
					    bool downto,
					    Expression*range_right);

/*
 * This function searches the currently active scope, or the global
 * scope, for the named type.
 */
extern const VType* parse_type_by_name(perm_string name);

extern void library_save_package(const char*libname, Package*pack);

extern void library_import(const YYLTYPE&loc, const std::list<perm_string>*names);

extern void library_use(const YYLTYPE&loc, ActiveScope*res, const char*libname, const char*pack, const char*ident);

extern void generate_global_types(ActiveScope*res);

#endif
