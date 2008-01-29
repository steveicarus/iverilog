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
#ident "$Id: memory.h,v 1.13 2007/03/22 16:08:19 steve Exp $"
#endif

#include "vvp_net.h"
#include "vpi_user.h"

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
extern void memory_attach_self(vvp_memory_t mem, vpiHandle self);

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
			    unsigned off,
			    vvp_vector4_t val);

/*
 * This doesn't actually write the value to the memory word, but
 * schedules for the write to happen some time in the future. The delay
 * is in simulation clock units.
 */
void schedule_memory(vvp_memory_t mem, unsigned addr,
		     vvp_vector4_t val, unsigned long delay);

/*
 * Get the word value at the given index into the memory.
 */
extern vvp_vector4_t memory_get_word(vvp_memory_t mem, unsigned idx);


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


/* vvp_fun_memport
 * The vvp_fum_memport is a structural port into a vvp_memory_t
 * object. The output is the word that is read from the addressed
 * memory, and the inputs are the address and optional write controls.
 *
 * 0  -- Address
 * This addresses the word in the memory. The output follows this
 * address as it changes, and also follows the value of the addressed
 * word.
 *
 * 1  -- Write event
 *
 * 2  -- Write enable
 *
 * 3  -- Write data
 *
 * NOTE: This functor is unique in that it needs to store the
 * vvp_net_t pointer associated with it. It needs this because it can
 * received input from other than its ports. Notably, the memory
 * itself reports word changes.
 */
class vvp_fun_memport  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_memport(vvp_memory_t mem, vvp_net_t*net);
      ~vvp_fun_memport();

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      vvp_memory_t mem_;

      friend void memory_set_word(vvp_memory_t, unsigned,
				  unsigned, vvp_vector4_t);
      void check_word_change(unsigned long address);
      class vvp_fun_memport*next_;

      unsigned long addr_;

      vvp_net_t*net_;
};

/*
**  Access to the memory symbol table.
**
** The memory_find function locates the memory device by name. If the
** device does not exist, a nil is returned.
**
** The memory_create function creates a new memory device with the given
** name. It is a fatal error to try to create a device that already exists.
*/
vvp_memory_t memory_find(char *label);
vvp_memory_t memory_create(char *label);

/*
 * $Log: memory.h,v $
 * Revision 1.13  2007/03/22 16:08:19  steve
 *  Spelling fixes from Larry
 *
 * Revision 1.12  2006/03/05 05:45:58  steve
 *  Add support for memory value change callbacks.
 *
 * Revision 1.11  2006/02/02 02:44:00  steve
 *  Allow part selects of memory words in l-values.
 *
 * Revision 1.10  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.9  2005/03/09 04:52:40  steve
 *  reimplement memory ports.
 *
 * Revision 1.8  2005/03/03 04:33:10  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 */

#endif
