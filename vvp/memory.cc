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
#ident "$Id: memory.cc,v 1.24 2005/03/05 05:44:32 steve Exp $"
#endif

#include "memory.h"
#include "symbols.h"
#include "schedule.h"
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
      vvp_memory_port_t port_list;
};

#define VVP_MEMORY_NO_ADDR ((int)0x80000000)

#if 0
struct vvp_memory_port_s : public functor_s
{
      void set(vvp_ipoint_t i, bool push, unsigned val, unsigned str);

      vvp_memory_t mem;
      vvp_ipoint_t ix;

      unsigned naddr;

      vvp_memory_port_t next;
      int cur_addr;
      vvp_memory_bits_t cur_bits;
      unsigned bitoff;
      unsigned nbits;

      bool writable;
};
#endif

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
	// XXXX For now, assume this can't happen
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

void memory_set_word(vvp_memory_t mem, unsigned addr, vvp_vector4_t val)
{
      memory_init_word(mem, addr, val);

      if (mem->port_list)
	    fprintf(stderr, "XXXX memory_set_word(%u, ...)"
		    " not fully implemented\n", addr);
}

#if 0
vvp_ipoint_t memory_port_new(vvp_memory_t mem,
			     unsigned nbits, unsigned bitoff,
			     unsigned naddr, bool writable)
{
  unsigned nfun = naddr;
  if (writable)
	nfun += 2 + nbits;
  nfun = (nfun+3)/4;
  if (nfun < nbits)
    nfun = nbits;

  vvp_memory_port_t a = new struct vvp_memory_port_s;

  a->mem = mem;
  a->naddr = naddr;
  a->writable = writable;
  a->nbits = nbits;
  a->bitoff = bitoff;
  a->next = mem->addr_root;
  mem->addr_root = a;

  a->ix = functor_allocate(nfun);
  functor_define(a->ix, a);

  if (nfun > 1)
    {
      extra_ports_functor_s *fu = new extra_ports_functor_s[nfun-1];
      for (unsigned i = 0; i< nfun - 1; i++) {
	fu[i].base_ = a->ix;
	functor_define(ipoint_index(a->ix, i+1), fu+i);
      }
    }

  a->cur_addr = VVP_MEMORY_NO_ADDR;
  a->cur_bits = 0x0;

  return a->ix;
}
#endif

void schedule_memory(vvp_memory_t mem, unsigned addr,
		     vvp_vector4_t val, unsigned long delay)
{
      fprintf(stderr, "XXXX Forgot how to schedule memory write.\n");
}

// Utilities
#if 0
inline static
vvp_memory_bits_t get_word_ix(vvp_memory_t mem, unsigned idx)
{
  return mem->bits + idx*mem->fwidth;
}
#endif
#if 0
inline static
vvp_memory_bits_t get_word(vvp_memory_t mem, int addr)
{
  assert(mem->a_idxs==1);
  unsigned waddr = addr - mem->a_idx[0].first;

  if (waddr >= mem->size)
    return 0x0;

  return get_word_ix(mem, waddr);
}
#endif
#if 0
inline static
bool set_bit(vvp_memory_bits_t bits, int bit, unsigned char val)
{
  int ix =    bit/4;
  int ip = 2*(bit%4);
  bool r = ((bits[ix] >> ip) & 3) != val;
  bits[ix] = (bits[ix] &~ (3<<ip)) | ((val&3) << ip);
  return r;
}
#endif
#if 0
inline static
unsigned char get_nibble(vvp_memory_bits_t bits, int bit)
{
  if (!bits)
    return 0xaa;
  int ix = bit/4;
  return bits[ix];
}
#endif
#if 0
inline static
unsigned char get_bit(vvp_memory_bits_t bits, int bit)
{
  return (get_nibble(bits, bit) >> (2*(bit&3))) & 3;
}
#endif
#if 0
inline static
unsigned char functor_get_inputs(vvp_ipoint_t ip)
{
  functor_t fp = functor_index(ip);
  assert(fp);
  return fp->ival;
}
#endif
#if 0
inline static
unsigned char functor_get_input(vvp_ipoint_t ip)
{
  unsigned char bits = functor_get_inputs(ip);
  return (bits >> (2*ipoint_port(ip))) & 3;
}
#endif
#if 0
static
bool update_addr_bit(vvp_memory_port_t addr, vvp_ipoint_t ip)
{
  unsigned abit = ip - addr->ix;

  assert(abit >= 0  &&  abit < addr->naddr);

  int old = addr->cur_addr;

  int abval = functor_get_input(ip);
  if (abval>1)
    addr->cur_addr = VVP_MEMORY_NO_ADDR;
  else if (addr->cur_addr == VVP_MEMORY_NO_ADDR)
    update_addr(addr);
  else if (abval)
    addr->cur_addr |=  (1<<abit);
  else
    addr->cur_addr &=~ (1<<abit);

  addr->cur_bits = get_word(addr->mem, addr->cur_addr);

  return addr->cur_addr != old;
}
#endif

#if 0
static
void update_addr(vvp_memory_port_t addr)
{
  addr->cur_addr = 0;
  for (unsigned i=0; i < addr->naddr; i++)
    {
      update_addr_bit(addr, addr->ix+i);
      if (addr->cur_addr == VVP_MEMORY_NO_ADDR)
	break;
    }
}
#endif

#if 0
inline static
void update_data(vvp_memory_port_t data)
{
  assert(data);
  for (unsigned i=0; i < data->nbits; i++)
    {
      vvp_ipoint_t dx = ipoint_index(data->ix, i);
      functor_t df = functor_index(dx);
      unsigned char out = get_bit(data->cur_bits, i + data->bitoff);
      df->put_oval(out, true);
    }
}
#endif

#if 0
static
void update_data_ports(vvp_memory_t mem, vvp_memory_bits_t bits, int bit,
		       unsigned char val)
{
  if (!bits)
    return;

  vvp_memory_port_t a = mem->addr_root;
  while (a)
    {
      if (bits == a->cur_bits)
	{
	  unsigned i = bit - a->bitoff;
	  if (i < a->nbits)
	    {
	      vvp_ipoint_t ix = ipoint_index(a->ix, i);
	      functor_t df = functor_index(ix);
	      df->put_oval(val, true);
	    }
	}
      a = a->next;
    }
}
#endif

#if 0
static inline
void write_event(vvp_memory_port_t p)
{
  if (!p->cur_bits)
    return;

  unsigned we = functor_get_input(p->ix + p->naddr + 1);
  if (!we)
    return;

  for (unsigned i=0; i < p->nbits; i++)
    {
      unsigned val = functor_get_input(p->ix + p->naddr + 2 + i);
      if (set_bit(p->cur_bits, i + p->bitoff, val))
	{
	  // if a write would change the memory bit, but <we> is
	  // undefined (x or z), set the bit to x.
	  if (we > 1)
	    {
	      set_bit(p->cur_bits, i + p->bitoff, 2);
	      val = 2;
	    }
	  update_data_ports(p->mem, p->cur_bits, i + p->bitoff, val);
	}
    }
}
#endif

#if 0
void vvp_memory_port_s::set(vvp_ipoint_t i, bool, unsigned val, unsigned)
{
  // !attention! "i" may not correspond to "this"
  functor_t ifu = functor_index(i);
  ifu->put(i, val);

  if (i < ix+naddr)
    {
      if (update_addr_bit(this, i))
	update_data(this);
    }

  // port ix+naddr is the write clock.  If its input value is
  // undefined, we do asynchronous write.  Else any event on ix+naddr
  // is a valid write clock edge.  Connect an appropriate edge event
  // functor.

  if (i == ix+naddr
      || (writable && functor_get_input(ix+naddr) == 3))
    {
      assert(writable);
      write_event(this);
    }
}
#endif


// %set/mem
#if 0
void memory_set(vvp_memory_t mem, unsigned idx, unsigned char val)
{
  if (idx/4 >= (mem->size * mem->fwidth))
    return;

  if (!set_bit(mem->bits, idx, val))
    return;

  unsigned widx = idx/(4*mem->fwidth);
  unsigned bidx = idx%(4*mem->fwidth);

  update_data_ports(mem, get_word_ix(mem, widx), bidx, val);
}
#endif

// %load/mem
#if 0
unsigned memory_get(vvp_memory_t mem, unsigned idx)
{
  if (idx/4 >= (mem->size * mem->fwidth))
    return 2;

  return get_bit(mem->bits, idx);
}
#endif
// %assign/mem event scheduling

struct mem_assign_s: public vvp_gen_event_s
{
  union
  {
    vvp_memory_t mem;
    struct mem_assign_s *next;
  };
  unsigned long idx;
};

static struct mem_assign_s* ma_free_list = 0;

inline static struct mem_assign_s* ma_alloc()
{
  struct mem_assign_s* cur = ma_free_list;
  if (!cur)
    cur = (struct mem_assign_s*) malloc(sizeof(struct mem_assign_s));
  else
    ma_free_list = cur->next;

  return cur;
}

inline static void ma_free(struct mem_assign_s* cur)
{
  cur->next = ma_free_list;
  ma_free_list = cur;
}

#if 0
static void run_mem_assign(vvp_gen_event_t obj, unsigned char val)
{
  struct mem_assign_s *e = (struct mem_assign_s *) obj;
  memory_set(e->mem, e->idx, val);
  ma_free(e);
}
#endif

/*
 * $Log: memory.cc,v $
 * Revision 1.24  2005/03/05 05:44:32  steve
 *  Get read width of unitialized memory words right.
 *
 * Revision 1.23  2005/03/03 04:33:10  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 */
