#ifndef IVL_fp_api_H
#define IVL_fp_api_H
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  <string>

/*
 * This is the interface function that the user invokes to parse a
 * footprint file. The argument is the path to the element.
 */
extern int parse_fp_file(const std::string&file_path);

/*
 * The yyltype supports the passing of detailed source file location
 * information between the lexical analyzer and the parser. Defining
 * YYLTYPE compels the lexor to use this type and not something other.
 */
struct yyltype {
      unsigned first_line;
      yyltype() { first_line = 1; }
};
# define YYLTYPE struct yyltype

/*
 * Use this function during parse to generate error messages. The "loc"
 * is the location of the token that triggered the error, and the fmt
 * is printf-style format.
 */
extern void errormsg(const YYLTYPE&loc, const char*fmt, ...) __attribute__((format (printf, 2, 3)));

extern void sorrymsg(const YYLTYPE&loc, const char*fmt, ...) __attribute__((format (printf, 2, 3)));

extern void callback_fp_element(const struct fp_element_t&);

/*
 * Set this to a non-zero value to enable parser debug output.
 */
//extern int yydebug;

/*
 * The parser counts the errors that is handed in the parse_errors
 * variable. For a clean compile, this value should not change. (The
 * caller sets its initial value.) The sorrys are the count of
 * unsupported constructs that are encountered.
 */
//extern int parse_errors;
extern int parse_fp_sorrys;

#endif /* IVL_fp_api_H */
