#ifndef __parse_api_H
#define __parse_api_H
/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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
# include  "entity.h"

/*
 * The vlltype supports the passing of detailed source file location
 * information between the lexical analyzer and the parser. Defining
 * YYLTYPE compels the lexor to use this type and not something other.
 */
struct yyltype {
      unsigned first_line;
      unsigned first_column;
      unsigned last_line;
      unsigned last_column;
      const char*text;
};
# define YYLTYPE struct yyltype

/*
 * The reset_lexor function takes the fd and makes it the input file
 * for the lexor. The path argument is used in lexor/parser error messages.
 */
extern void reset_lexor(FILE*fd, const char*path);

/*
 * The parser calls yylex to get the next lexical token. It is only
 * called by the bison-generated parser.
 */
extern int yylex(void);

/*
 * This is the bison-generated parser.
 */
extern int yyparse(void);

/*
 * Use this functio during parse to generate error messages. The "loc"
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

#endif
