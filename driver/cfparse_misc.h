#ifndef __cfparse_misc_H
#define __cfparse_misc_H
/*
 * Copyright (c) 2001 Picture Elements, Inc.
 *    Stephen Williams (steve@picturel.com)
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
#ident "$Id: cfparse_misc.h,v 1.6 2004/02/15 18:03:30 steve Exp $"
#endif

/*
 * The vlltype supports the passing of detailed source file location
 * information between the lexical analyzer and the parser. Defining
 * YYLTYPE compels the lexor to use this type and not something other.
 */
struct cfltype {
      unsigned first_line;
      unsigned first_column;
      unsigned last_line;
      unsigned last_column;
      const char*text;
};
# define YYLTYPE struct cfltype
extern YYLTYPE yylloc;

int cflex(void);
int cferror(const char *);
int cfparse(void);

/*
 * $Log: cfparse_misc.h,v $
 * Revision 1.6  2004/02/15 18:03:30  steve
 *  Cleanup of warnings.
 *
 * Revision 1.5  2003/09/26 21:25:58  steve
 *  Warnings cleanup.
 *
 * Revision 1.4  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/01/02 02:39:34  steve
 *  Use my own cfltype to defend against bison 1.30.
 *
 * Revision 1.2  2001/11/12 18:47:32  steve
 *  Support +incdir in command files, and ignore other
 *  +args flags. Also ignore -a and -v flags.
 *
 * Revision 1.1  2001/11/12 01:26:36  steve
 *  More sophisticated command file parser.
 *
 */
#endif
