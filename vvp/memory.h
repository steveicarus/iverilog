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
#ident "$Id: memory.h,v 1.8 2005/03/03 04:33:10 steve Exp $"
#endif

#include "vvp_net.h"

/*
**  vvp_memory_t         is a memory
**  vvp_memory_index_t   is a memory index range definition
*/
typedef struct vvp_memory_s *vvp_memory_t;

struct memory_address_range {
      int msb;
      int lsb;
};

/*
 * Given a memory device, the memory_configure function configures it
 * by defining the dimensions of the device. It is an error to
 * redefine the dimensions of a device already configured.
 *
 * The lsb and msb define the dimensions of a word. They are in
 * Verilog form. The actual word contents are vvp_vector4_t values.
 *
 * The idx array is an array of address ranges that describe the
 * complete multi-dimensional array. In a normal Verilog array, idxs
 * is 1, and idx is a pointer to a single memory_address_range. The
 * table does not need to be persistent.
 */
extern void memory_configure(vvp_memory_t mem, int msb, int lsb,
			     unsigned idxs,
			     const struct memory_address_range *idx);

/*
 * init_word and set_word functions take the memory to be manipulated
 * and write a word value at the given word address. The idx is the
 * canonical (0-based, 1-dimensional) address of the word to be
 * written. The caller needs to have converted any multi-dimensional
 * address into a canonical address first.
 *
 * The difference between init_word and set_word are that the set_word
 * function causes values to be propagated through structural ports,
 * but the init_word does not.
 */
extern void memory_init_word(vvp_memory_t mem,
			     unsigned idx,
			     vvp_vector4_t val);
extern void memory_set_word(vvp_memory_t mem,
			    unsigned idx,
			    vvp_vector4_t val);

/*
 * this doesn't actually write the value to the memory word, but
 * scedules for the write to happen some time in the future. The delay
 * is in simulation clock units
 */
void schedule_memory(vvp_memory_t mem, unsigned addr,
		     vvp_vector4_t val, unsigned long delay);

/*
 * Get the word value at the given index into the memory.
 */
extern vvp_vector4_t memory_get_word(vvp_memory_t mem, unsigned idx);

#if 0
vvp_ipoint_t memory_port_new(vvp_memory_t mem,
			     unsigned nbits, unsigned bitoff,
			     unsigned naddr, bool writable);
#endif


  /* Number of words in the memory. */
unsigned memory_word_count(vvp_memory_t mem);
  /* Width of a word */
unsigned memory_word_width(vvp_memory_t mem);
  /* Get the user declared geometry of the memory address. This is the
     msb and lsb values for each pair in the multi-dimensional array. */
long memory_left_range(vvp_memory_t mem, unsigned ix);
long memory_right_range(vvp_memory_t mem, unsigned ix);
  /* Get the user defined geometry for the memory *word*. */
long memory_word_left_range(vvp_memory_t mem);
long memory_word_right_range(vvp_memory_t mem);

/*
**  Access to the memory symbol table.
**
** The memory_find function locates the memory device by name. If the
** device does not exist, a nil is returned.
**
** The memory_create functio create a new memory device with the given
** name. It is a fatal error to try to create a device that already exists.
*/
vvp_memory_t memory_find(char *label);
vvp_memory_t memory_create(char *label);

/*
 * $Log: memory.h,v $
 * Revision 1.8  2005/03/03 04:33:10  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 */

#endif
