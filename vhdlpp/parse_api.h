#ifndef IVL_parse_api_H
#define IVL_parse_api_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
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
# include  "entity.h"

typedef void*yyscan_t;

/*
 * The yyltype supports the passing of detailed source file location
 * information between the lexical analyzer and the parser. Defining
 * YYLTYPE compels the lexor to use this type and not something other.
 */
struct yyltype {
      unsigned first_line;
      const char*text;
      yyltype() { first_line = 1; text = ""; }
};
# define YYLTYPE struct yyltype

/*
 * This calls the bison-generated parser with the given file path as
 * the input stream. If the file cannot be opened, this returns -1.
 * The "library_name" argument is the name of the library that is
 * being parsed. If this is a regular source file, then this name is
 * nil. Note that the "work" library is handled specially.
 */
extern int parse_source_file(const char*file_path, perm_string library_name);

/*
 * Use this function during parse to generate error messages. The "loc"
 * is the location of the token that triggered the error, and the fmt
 * is printf-style format.
 */
extern void errormsg(const YYLTYPE&loc, const char*fmt, ...) __attribute__((format (printf, 2, 3)));

extern void sorrymsg(const YYLTYPE&loc, const char*fmt, ...) __attribute__((format (printf, 2, 3)));

/*
 * Set this to a non-zero value to enable parser debug output.
 */
extern int yydebug;

/*
 * The parser counts the errors that is handed in the parse_errors
 * variable. For a clean compile, this value should not change. (The
 * caller sets its initial value.) The sorrys are the count of
 * unsupported constructs that are encountered.
 */
extern int parse_errors;
extern int parse_sorrys;

#endif /* IVL_parse_api_H */
