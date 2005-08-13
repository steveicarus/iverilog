#ifndef __util_H
#define __util_H
/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: util.h,v 1.7.2.1 2005/08/13 00:45:55 steve Exp $"
#endif

# include  <map>
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
extern bool load_module(const char*type);



struct attrib_list_t {
      perm_string key;
      verinum val;
};

extern attrib_list_t* evaluate_attributes(const std::map<perm_string,PExpr*>&att,
					  unsigned&natt,
					  const Design*des,
					  const NetScope*scope);

/*
 * $Log: util.h,v $
 * Revision 1.7.2.1  2005/08/13 00:45:55  steve
 *  Fix compilation warnings/errors with newer compilers.
 *
 * Revision 1.7  2004/02/20 18:53:36  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.6  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/05/23 03:08:52  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.4  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.3  2001/10/20 23:02:40  steve
 *  Add automatic module libraries.
 */
#endif
