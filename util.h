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
#ident "$Id: util.h,v 1.3 2001/10/20 23:02:40 steve Exp $"
#endif

# include  <string>

/*
 * This function returns the first name of a hierarchical path, and
 * sets the parameter to what's left. If there is no path, then the
 * parameter is set to "".
 */
extern string parse_first_name(string&path);

extern string parse_last_name(string&path);

/*
 * This file attempts to locate a module in a file. It operates by
 * looking for a plausible Verilog file to hold the module, and
 * invoking the parser to bring in that file's contents.
 */
extern bool load_module(const char*type);


/*
 * $Log: util.h,v $
 * Revision 1.3  2001/10/20 23:02:40  steve
 *  Add automatic module libraries.
 *
 * Revision 1.2  2001/01/14 23:04:56  steve
 *  Generalize the evaluation of floating point delays, and
 *  get it working with delay assignment statements.
 *
 *  Allow parameters to be referenced by hierarchical name.
 *
 * Revision 1.1  2000/04/29 04:53:44  steve
 *  missing header file.
 *
 */
#endif
