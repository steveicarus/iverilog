#ifndef __util_H
#define __util_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: util.h,v 1.4 2001/12/03 04:47:15 steve Exp $"
#endif


/*
 * This file attempts to locate a module in a file. It operates by
 * looking for a plausible Verilog file to hold the module, and
 * invoking the parser to bring in that file's contents.
 */
extern bool load_module(const char*type);


/*
 * $Log: util.h,v $
 * Revision 1.4  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.3  2001/10/20 23:02:40  steve
 *  Add automatic module libraries.
 */
#endif
