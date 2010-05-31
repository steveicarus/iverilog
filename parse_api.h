#ifndef __parse_api_H
#define __parse_api_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  <cstdio>
# include  "StringHeap.h"
# include  <string>
# include  <map>

class Module;
class PUdp;

/*
 * These are maps of the modules and primitives parsed from the
 * Verilog source into pform for elaboration. The parser adds modules
 * to these maps as it compiles modules in the Verilog source.
 */
extern map<perm_string,Module*> pform_modules;
extern map<perm_string,PUdp*>   pform_primitives;

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

#endif
