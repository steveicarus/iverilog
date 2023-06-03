#ifndef IVL_parse_api_H
#define IVL_parse_api_H
/*
 * Copyright (c) 2001-2021 Stephen Williams (steve@icarus.com)
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
extern std::vector<PPackage*>        pform_units;
extern std::vector<PPackage*>        pform_packages;

extern void pform_dump(std::ostream&out, const PClass*pac);
extern void pform_dump(std::ostream&out, const PPackage*pac);
extern void pform_dump(std::ostream&out, const PTaskFunc*tf);

/*
 * This code actually invokes the parser to make modules. If the path
 * parameter is "-", the parser reads from stdin, otherwise it attempts
 * to open and read the specified file. When reading from a file, if
 * the ivlpp_string variable is not set to null, the file will be piped
 * through the command specified by ivlpp_string before being parsed.
 */
extern int pform_parse(const char*path);

extern void pform_finish();

extern std::string vl_file;

extern void pform_set_timescale(int units, int prec, const char*file,
                                unsigned lineno);
extern int def_ts_units;
extern int def_ts_prec;

#endif /* IVL_parse_api_H */
