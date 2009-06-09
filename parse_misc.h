#ifndef __parse_misc_H
#define __parse_misc_H
/*
 * Copyright (c) 1998-2009 Stephen Williams (steve@icarus.com)
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

# include  <list>
# include  <ostream>
# include  "compiler.h"
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

class LineInfo;
inline void FILE_NAME(LineInfo*tmp, const struct vlltype&where)
{
      tmp->set_lineno(where.first_line);
      tmp->set_file(filename_strings.make(where.text));
}

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

extern void destroy_lexor();

extern ostream& operator << (ostream&, const YYLTYPE&loc);

extern unsigned error_count, warn_count;
extern unsigned long based_size;

extern bool in_celldefine;
enum UCDriveType { UCD_NONE, UCD_PULL0, UCD_PULL1 };
extern UCDriveType uc_drive;

#endif
