#ifndef __parse_misc_H
#define __parse_misc_H
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
#if !defined(WINNT)
#ident "$Id: parse_misc.h,v 1.1 2001/03/11 00:29:39 steve Exp $"
#endif

/*
 * This method is called to compile the design file. The input is read
 * and a list of statements is created.
 */
extern int compile_design(const char*path);

/*
 * various functions shared by the lexor and the parser.
 */
extern int yylex(void);
extern void yyerror(const char*msg);

/*
 * This is the path of the current source file.
 */
extern const char*yypath;
extern unsigned yyline;

struct textv_s {
      unsigned cnt;
      char**text;
};

extern void textv_init(struct textv_s*obj);
extern void textv_add(struct textv_s*obj, char*item);

/*
 * $Log: parse_misc.h,v $
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
