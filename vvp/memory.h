#ifndef __memory_H                                      // -*- c++ -*- 
#define __memory_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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
#ident "$Id: memory.h,v 1.6 2002/08/12 01:35:08 steve Exp $"
#endif

#include "pointers.h"
#include "functor.h"

/*
**  vvp_memory_t         is a memory
**  vvp_memory_bits_t    are bits in a memory
**  vvp_memory_index_t   is a memory index range definition
*/
typedef struct vvp_memory_s *vvp_memory_t;
typedef unsigned char *vvp_memory_bits_t;
typedef struct vvp_memory_index_s *vvp_memory_index_t;

void memory_new(vvp_memory_t mem, char *name, int lsb, int msb,
		unsigned idxs, long *idx);
vvp_ipoint_t memory_port_new(vvp_memory_t mem,
			     unsigned nbits, unsigned bitoff,
			     unsigned naddr, bool writable);

void memory_init_nibble(vvp_memory_t mem, unsigned idx, unsigned char val);

void memory_set(vvp_memory_t mem, unsigned idx, unsigned char val);
unsigned memory_get(vvp_memory_t mem, unsigned idx);
void schedule_memory(vvp_memory_t mem, unsigned idx, 
		     unsigned char val, unsigned delay);

unsigned memory_size(vvp_memory_t mem);
char *memory_name(vvp_memory_t mem);
unsigned memory_data_width(vvp_memory_t mem);
unsigned memory_root(vvp_memory_t mem, unsigned ix = 0);
unsigned memory_left_range(vvp_memory_t mem, unsigned ix = 0);
unsigned memory_right_range(vvp_memory_t mem, unsigned ix = 0);
unsigned memory_word_left_range(vvp_memory_t mem);
unsigned memory_word_right_range(vvp_memory_t mem);

/*
**  Access to the memory symbol table.
*/
vvp_memory_t memory_find(char *label);
vvp_memory_t memory_create(char *label);

/*
 * $Log: memory.h,v $
 * Revision 1.6  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/01/31 04:28:17  steve
 *  Full support for $readmem ranges (Tom Verbeure)
 *
 * Revision 1.4  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.3  2001/06/15 03:28:31  steve
 *  Change the VPI call process so that loaded .vpi modules
 *  use a function table instead of implicit binding.
 *
 * Revision 1.2  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.1  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
 *
 */

#endif
