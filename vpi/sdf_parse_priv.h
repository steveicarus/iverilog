#ifndef IVL_sdf_parse_priv_h
#define IVL_sdf_parse_priv_h
/*
 * Copyright (c) 2007-2014 Stephen Williams (steve@icarus.com)
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

/*
 * This file is only included by sdf_parse.y and sdf_lexor.lex. It is
 * used to share declarations between the parse and the lexor.
 */

  /* Path to source for error messages. */
extern const char*sdf_parse_path;

/* Hierarchy separator character to use. */
extern char sdf_use_hchar;

extern void start_edge_id(unsigned cond);
extern void stop_edge_id(void);

#endif /* IVL_sdf_parse_priv_h */
