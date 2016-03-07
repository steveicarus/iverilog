#ifndef IVL_parse_misc_H
#define IVL_parse_misc_H
/*
 * Copyright (c) 2011,2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "parse_api.h"

class ActiveScope;
class Architecture;
class Expression;
class Package;
class ExpRange;
class ExpString;
class ScopeBase;
class VType;

extern void bind_entity_to_active_scope(const char*ename, ActiveScope*scope);
extern void bind_architecture_to_entity(const char*ename, Architecture*arch);

extern const VType* calculate_subtype_array(const YYLTYPE&loc, const char*base_name,
					    ScopeBase*scope,
					    std::list<ExpRange*>*ranges);
extern const VType* calculate_subtype_range(const YYLTYPE&loc, const char*base_name,
					    ScopeBase*scope,
					    Expression*range_left,
					    int direction,
					    Expression*range_right);

/*
 * This function searches the currently active scope, or the global
 * scope, for the named type.
 */
extern const VType* parse_type_by_name(perm_string name);

/*
 * The parser calls the library_save_package function when it parses a
 * package. The library_parse_name is the name of the library that is
 * currently being processed (by a recursive call to the parser to
 * load a package from a library) or a nil name to indicate that this
 * is from the live parser.
 */
extern void library_save_package(perm_string library_parse_name, Package*pack);

extern Package*library_recall_package(perm_string library_parse_name, perm_string name);

extern void library_import(const YYLTYPE&loc, const std::list<perm_string>*names);

extern void library_use(const YYLTYPE&loc, ActiveScope*res, const char*libname, const char*pack, const char*ident);

/*
 * Converts CHARACTER enums to an ExpString* if applicable.
 * See the standard VHDL library (package STANDARD) or VHDL-2008/16.3
 * for more details).
 */
extern ExpString*parse_char_enums(const char*str);

#endif /* IVL_parse_misc_H */
