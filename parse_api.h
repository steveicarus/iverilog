#ifndef __parse_api_H
#define __parse_api_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: parse_api.h,v 1.3.2.1 2005/08/13 00:45:54 steve Exp $"
#endif

# include  <stdio.h>
# include  "StringHeap.h"
# include  <string>
# include  <map>

class Module;
class PUdp;

/*
 * These are maps of the modules and primitives parsed from the
 * Verilog source into pform for elaboration. The parser adds modules
 * to these maps as it compiles modules in the verilog source.
 */
extern std::map<perm_string,Module*> pform_modules;
extern std::map<perm_string,PUdp*>   pform_primitives;

/*
 * This code actually invokes the parser to make modules. The first
 * parameter is the name of the file that is to be parsed. The
 * optional second parameter is the opened descriptor for the file. If
 * the descriptor is 0 (or skipped) then the function will attempt to
 * open the file on its own.
 */
extern int  pform_parse(const char*path, FILE*file =0);

extern std::string vl_file;

/*
 * $Log: parse_api.h,v $
 * Revision 1.3.2.1  2005/08/13 00:45:54  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.3  2004/02/18 17:11:57  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.2  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2001/10/21 20:18:56  steve
 *  clean up API for restarting parser.
 *
 */
#endif
