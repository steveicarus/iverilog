#ifndef __memory_H                                      // -*- c++ -*-
#define __memory_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

// moved from memory.cc for callback function visibility
typedef struct vvp_memory_port_s *vvp_memory_port_t;

// moved here from memory.cc for callback function visibility
struct vvp_memory_s
{
  char *name;                     // VPI scope.name

  // Address port properties:
  unsigned size;                  // total number of data words
  unsigned a_idxs;                // number of address indices
  vvp_memory_index_t a_idx;       // vector of address indices

  // Data port properties:
  unsigned width;                 // number of data bits
  unsigned fwidth;                // number of bytes (4bits) per data word
  int msb, lsb;                   // Most/Least Significant data bit (VPI)

  vvp_memory_bits_t bits;         // Array of bits
  vvp_memory_port_t addr_root;    // Port list root;

  // callbacks
  struct __vpiCallback*cb;        // callback list for this vpiMemory
};

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

#endif
