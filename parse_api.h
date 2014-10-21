#ifndef IVL_parse_api_H
#define IVL_parse_api_H
/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

# include  <cstdio>
# include  "StringHeap.h"
# include  <string>
# include  <ostream>
# include  <map>
# include  <set>

class Design;
class Module;
class PClass;
class PPackage;
class PTaskFunc;
class PUdp;
class data_type_t;
struct enum_type_t;

/*
 * These are maps of the modules and primitives parsed from the
 * Verilog source into pform for elaboration. The parser adds modules
 * to these maps as it compiles modules in the Verilog source.
 */
extern std::map<perm_string,Module*> pform_modules;
extern std::map<perm_string,PUdp*>   pform_primitives;
extern std::map<perm_string,data_type_t*> pform_typedefs;
extern std::set<enum_type_t*> pform_enum_sets;
extern std::map<perm_string,PTaskFunc*> pform_tasks;
extern std::map<perm_string,PClass*> pform_classes;
extern std::map<perm_string,PPackage*> pform_packages;

extern void pform_dump(std::ostream&out, const PClass*pac);
extern void pform_dump(std::ostream&out, const PPackage*pac);
extern void pform_dump(std::ostream&out, const PTaskFunc*tf);

extern void elaborate_rootscope_enumerations(Design*des);
extern void elaborate_rootscope_classes(Design*des);
extern void elaborate_rootscope_tasks(Design*des);

/*
 * This code actually invokes the parser to make modules. The first
 * parameter is the name of the file that is to be parsed. The
 * optional second parameter is the opened descriptor for the file. If
 * the descriptor is 0 (or skipped) then the function will attempt to
 * open the file on its own.
 */
extern int  pform_parse(const char*path, FILE*file =0);

extern string vl_file;

extern void pform_set_timescale(int units, int prec, const char*file,
                                unsigned lineno);
extern int def_ts_units;
extern int def_ts_prec;

#endif /* IVL_parse_api_H */
