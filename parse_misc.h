#ifndef IVL_parse_misc_H
#define IVL_parse_misc_H
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
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
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      unsigned lexical_pos;
      const char*text;
      std::string get_fileline() const;
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
extern void VLerror(const YYLTYPE&loc, const char*msg, ...) __attribute__((format(printf,2,3)));
#define yywarn VLwarn
extern void VLwarn(const char*msg);
extern void VLwarn(const YYLTYPE&loc, const char*msg);

extern void destroy_lexor();

extern std::ostream& operator << (std::ostream&, const YYLTYPE&loc);

extern unsigned error_count, warn_count;
extern unsigned long based_size;

extern bool in_celldefine;
enum UCDriveType { UCD_NONE, UCD_PULL0, UCD_PULL1 };
extern UCDriveType uc_drive;

/*
 * The parser signals back to the lexor that the next identifier
 * should be in the package scope. For example, if the source is
 *    <package> :: <foo>
 * Then the parser calls this function to set the package context so
 * that the lexor can interpret <foo> in the package context.
 */
extern void lex_in_package_scope(PPackage*pkg);

/*
 * Test if this identifier is a type identifier in the current
 * context. The pform code needs to help the lexor here because the
 * parser detects typedefs and marks the typedef'ed identifiers as
 * type names.
 */
extern typedef_t* pform_test_type_identifier(const YYLTYPE&loc, const char*txt);
extern typedef_t* pform_test_type_identifier(PPackage*pkg, const char*txt);

/*
 * Test if this identifier is a package name. The pform needs to help
 * the lexor here because the parser detects packages and saves them.
 */
extern PPackage* pform_test_package_identifier(const char*txt);

/*
 * Export these functions because we have to generate PENumber class
 * in pform.cc for user defparam definition from command file.
 */
extern verinum*make_unsized_dec(const char*txt);
extern verinum*make_undef_highz_dec(const char*txt);
extern verinum*make_unsized_binary(const char*txt);
extern verinum*make_unsized_octal(const char*txt);
extern verinum*make_unsized_hex(const char*txt);

extern char* strdupnew(char const *str);

#endif /* IVL_parse_misc_H */
