#ifndef __sys_readmem_lex_H
#define __sys_readmem_lex_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: sys_readmem_lex.h,v 1.4 2002/08/12 01:35:05 steve Exp $"
#endif

# include  <stdio.h>
# include  "vpi_user.h"

# define MEM_ADDRESS 257
# define MEM_WORD    258

extern void sys_readmem_start_file(FILE*in, int bin_flag,
				   unsigned width, struct t_vpi_vecval*val);
extern int readmemlex();

/*
 * $Log: sys_readmem_lex.h,v $
 * Revision 1.4  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.2  2000/01/23 23:54:36  steve
 *  Compile time problems with vpi_user.h
 *
 * Revision 1.1  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 */
#endif
