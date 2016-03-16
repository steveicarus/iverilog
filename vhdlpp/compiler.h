#ifndef IVL_compiler_H
#define IVL_compiler_H
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

# include  "StringHeap.h"
# include  <fstream>

const int GN_KEYWORD_2008  = 0x0001;

// TRUE if processing is supposed to dump progress to stderr.
extern bool verbose_flag;

extern bool debug_elaboration;
extern std::ofstream debug_log_file;

// Stores strings created by the lexer
extern StringHeapLex lex_strings;

// Stores file names
extern StringHeapLex filename_strings;

// Stores generated strings (e.g. scope names)
extern StringHeapLex gen_strings;

#endif /* IVL_compiler_H */
