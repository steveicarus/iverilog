#ifndef __parse_misc_H
#define __parse_misc_H
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: parse_misc.h,v 1.7 2003/03/08 20:58:18 steve Exp $"
#endif

# include  <list>
# include  "pform.h"

/*
 * The vlltype supports the passing of detailed source file location
 * information between the lexical analyzer and the parser. Defining
 * YYLTYPE compels the lexor to use this type and not something other.
 */
struct vlltype {
      unsigned first_line;
      unsigned first_column;
      unsigned last_line;
      unsigned last_column;
      const char*text;
};
# define YYLTYPE struct vlltype

  /* This for compatibility with new and older bison versions. */
#ifndef yylloc
# define yylloc VLlloc
#endif
extern YYLTYPE yylloc;

/*
 * Interface into the lexical analyzer. ...
 */
extern int  VLlex();
extern void VLerror(const char*msg);
extern void VLerror(const YYLTYPE&loc, const char*msg);
#define yywarn VLwarn
extern void VLwarn(const YYLTYPE&loc, const char*msg);

extern unsigned error_count, warn_count;

/*
 * $Log: parse_misc.h,v $
 * Revision 1.7  2003/03/08 20:58:18  steve
 *  More C-like use of vlltype.
 *
 * Revision 1.6  2002/11/03 20:36:53  steve
 *  Support old/new bison yylloc.
 *
 * Revision 1.5  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.3  1999/07/10 01:03:18  steve
 *  remove string from lexical phase.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:29:03  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
