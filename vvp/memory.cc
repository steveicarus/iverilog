/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: memory.cc,v 1.29 2006/03/05 05:45:58 steve Exp $"
#endif

#include "memory.h"
#include "symbols.h"
#include "schedule.h"
#include "vpi_priv.h"
#include <assert.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <string.h>


typedef struct vvp_memory_port_s *vvp_memory_port_t;

struct vvp_memory_s
{
	// Address ranges (1 or more)
      unsigned nrange;
      struct memory_address_range*ranges;

	// Data port properties:
      unsigned width;                 // number of data bits

      int msb, lsb;                   // Most/Least Significant data bit (VPI)

	// Array of words.
      unsigned word_count;
      vvp_vector4_t*words;

	// List of ports into this memory.
      class vvp_fun_memport* port_list;
      vpiHandle vpi_self;
};

#define VVP_MEMORY_NO_ADDR ((int)0x80000000)

// Compilation

static symbol_table_t memory_table = 0;

vvp_memory_t memory_find(char *label)
{
      if (memory_table == 0)
	    return 0;

      symbol_value_t v = sym_get_value(memory_table, label);
      return (vvp_memory_t)v.ptr;
}

vvp_memory_t memory_create(char *label)
{
      if (!memory_table)
	    memory_table = new_symbol_table();

      assert(!memory_find(label));

      vvp_memory_t mem = new struct vvp_memory_s;

      symbol_value_t v;
      v.ptr = mem;
      sym_set_value(memory_table, label, v);

      return mem;
}

void memory_configure(vvp_memory_t mem,
		      int msb, int lsb,
		      unsigned nrange,
		      const struct memory_address_range*ranges)
{
	/* Get the word width details. */
      mem->width = msb > lsb ? msb-lsb+1 : lsb-msb+1;
      mem->msb = msb;
      mem->lsb = lsb;

	/* Make a private copy of the memory address ranges. */
      assert(nrange > 0);
      mem->nrange = nrange;
      mem->ranges = new struct memory_address_range[nrange];
      for (unsigned idx = 0 ;  idx < nrange ;  idx += 1)
	    mem->ranges[idx] = ranges[idx];

	/* Scan the indices (multiplying each range) to add up the
	   total number of words in this memory. */
      mem->word_count = 1;
      for (unsigned idx = 0 ;  idx < mem->nrange ;  idx += 1) {
	    struct memory_address_range*rp = mem->ranges+idx;

	    unsigned count = rp->msb > rp->lsb
		  ? rp->msb - rp->lsb + 1
		  : rp->lsb - rp->msb + 1;

	    mem->word_count *= count;
      }

      mem->words = new vvp_vector4_t [mem->word_count];
      assert(mem->words);

      mem->port_list = 0;
      mem->vpi_self  = 0;
}

void memory_attach_self(vvp_memory_t mem, vpiHandle self)
{
      assert(mem->vpi_self == 0);
      mem->vpi_self = self;
}

unsigned memory_word_width(vvp_memory_t mem)
{
      return mem->width;
}

unsigned memory_word_count(vvp_memory_t mem)
{
      return mem->word_count;
}

long memory_word_left_range(vvp_memory_t mem)
{
      return mem->msb;
}

long memory_word_right_range(vvp_memory_t mem)
{
      return mem->lsb;
}

long memory_left_range(vvp_memory_t mem, unsigned ix)
{
      assert(ix < mem->nrange);
      return mem->ranges[ix].msb;
}

long memory_right_range(vvp_memory_t mem, unsigned ix)
{
      assert(ix < mem->nrange);
      return mem->ranges[ix].lsb;
}

vvp_vector4_t memory_get_word(vvp_memory_t mem, unsigned addr)
{
	/* If the address is out of range, then return a vector of all
	   X bits. */
      if (addr >= mem->word_count) {
	    vvp_vector4_t val (mem->width);
	    for (unsigned idx = 0 ;  idx < mem->width ;  idx += 1)
		  val.set_bit(idx, BIT4_X);
	    return val;
      }

      assert(addr <= mem->word_count);

      if (mem->words[addr].size() == 0) {
	    vvp_vector4_t tmp (mem->width);
	    for (unsigned idx = 0 ;  idx < mem->width ;  idx += 1)
		  tmp.set_bit(idx, BIT4_X);
	    mem->words[addr] = tmp;
      }

      return mem->words[addr];
}

void memory_init_word(vvp_memory_t mem, unsigned addr, vvp_vector4_t val)
{
      if (addr >= mem->word_count)
	    return;

      assert(val.size() == mem->width);
      mem->words[addr] = val;
}

void memory_set_word(vvp_memory_t mem, unsigned addr,
		     unsigned off, vvp_vector4_t val)
{
      if (addr >= mem->word_count)
	    return;

      if (off >= mem->width)
	    return;

      if (off == 0 && val.size() == mem->width) {
	    mem->words[addr] = val;

      } else {
	    if ((off + val.size()) > mem->width)
		  val = val.subvalue(0, mem->width - off);

	    mem->words[addr].set_vec(off, val);
      }

      for (vvp_fun_memport*cur = mem->port_list
		 ; cur ;  cur = cur->next_) {
	    cur->check_word_change(addr);
      }

      vpip_run_memory_value_change(mem->vpi_self, addr);
}

void schedule_memory(vvp_memory_t mem, unsigned addr,
		     vvp_vector4_t val, unsigned long delay)
{
      fprintf(stderr, "XXXX Forgot how to schedule memory write.\n");
}

vvp_fun_memport::vvp_fun_memport(vvp_memory_t mem, vvp_net_t*net)
: mem_(mem), net_(net)
{
      addr_ = 0;
      next_ = mem_->port_list;
      mem_->port_list = this;
}

vvp_fun_memport::~vvp_fun_memport()
{
}

void vvp_fun_memport::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      bool addr_valid_flag;

      switch (port.port()) {

	  case 0: // Address input
	    addr_valid_flag = vector4_to_value(bit, addr_);
	    if (! addr_valid_flag)
		  addr_ = memory_word_count(mem_);
	    vvp_send_vec4(port.ptr()->out, memory_get_word(mem_,addr_));
	    break;

	  default:
	    fprintf(stdout, "XXXX write ports not implemented.\n");
	    assert(0);
      }
}

/*
 * This function is called by the memory itself to tell this port that
 * the given address had a content change. The device itself figures
 * out what to do with that information.
 */
void vvp_fun_memport::check_word_change(unsigned long addr)
{
      if (addr != addr_)
	    return;

      vvp_vector4_t bit = memory_get_word(mem_, addr_);
      vvp_send_vec4(net_->out, bit);
}


/*
 * $Log: memory.cc,v $
 * Revision 1.29  2006/03/05 05:45:58  steve
 *  Add support for memory value change callbacks.
 *
 * Revision 1.28  2006/02/02 02:44:00  steve
 *  Allow part selects of memory words in l-values.
 *
 * Revision 1.27  2005/06/22 00:04:49  steve
 *  Reduce vvp_vector4 copies by using const references.
 *
 * Revision 1.26  2005/03/09 04:52:40  steve
 *  reimplement memory ports.
 *
 * Revision 1.25  2005/03/06 17:07:48  steve
 *  Non blocking assign to memory words.
 *
 * Revision 1.24  2005/03/05 05:44:32  steve
 *  Get read width of unitialized memory words right.
 *
 * Revision 1.23  2005/03/03 04:33:10  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 */
