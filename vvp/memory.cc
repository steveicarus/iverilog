/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

#include "vpi_priv.h"
#include "memory.h"
#include "symbols.h"
#include "schedule.h"
#include <assert.h>
#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif
#include <stdlib.h>
#include <string.h>

#if 0
typedef struct vvp_memory_port_s *vvp_memory_port_t;

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
#endif

unsigned memory_data_width(vvp_memory_t mem)
{
  return mem->width;
}

#define VVP_MEMORY_NO_ADDR ((int)0x80000000)

struct vvp_memory_index_s
{
  int first;       // first memory address
  unsigned size;   // number of valid addresses

  // Added to correctly support vpiLeftRange and vpiRightRange
  int left;
  int right;
};

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

unsigned memory_size(vvp_memory_t mem)
{
  return mem->size;
}

unsigned memory_root(vvp_memory_t mem, unsigned ix)
{
      if (ix >= mem->a_idxs)
	    return 0;
      return mem->a_idx[ix].first;
}

unsigned memory_left_range(vvp_memory_t mem, unsigned ix)
{
      if (ix >= mem->a_idxs)
	    return 0;
      return mem->a_idx[ix].left;
}

unsigned memory_right_range(vvp_memory_t mem, unsigned ix)
{
      if (ix >= mem->a_idxs)
	    return 0;
      return mem->a_idx[ix].right;
}

unsigned memory_word_left_range(vvp_memory_t mem)
{
      return mem->msb;
}

unsigned memory_word_right_range(vvp_memory_t mem)
{
      return mem->lsb;
}

char *memory_name(vvp_memory_t mem)
{
      return mem->name;
}

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
  mem->cb = NULL; // clear the callbacks
  return mem;
}

void memory_new(vvp_memory_t mem, char *name, int msb, int lsb,
		unsigned idxs, long *idx)
{
  mem->width = msb > lsb ? msb-lsb+1 : lsb-msb+1;
  mem->msb = msb;
  mem->lsb = lsb;
  mem->fwidth = (mem->width+3)/4;

  assert((idxs&1) == 0);
  mem->a_idxs = idxs/2;
  mem->a_idx = (vvp_memory_index_t)
    malloc(mem->a_idxs*sizeof(struct vvp_memory_index_s));
  assert(mem->a_idxs);

  mem->size = 1;
  for (unsigned i=0; i < mem->a_idxs; i++)
    {
      vvp_memory_index_t x = mem->a_idx + i;
      int msw = *(idx++);
      int lsw = *(idx++);

      x->left = msw;
      x->right = lsw;

      if (msw > lsw) {
	    x->size  = msw - lsw + 1;
	    x->first = lsw;
      }
      else {
	    x->size  = lsw - msw + 1;
	    x->first = msw;
      }
      mem->size *= x->size;
    }

  mem->bits  = (vvp_memory_bits_t) malloc(mem->size * mem->fwidth);
  assert(mem->bits);
  memset(mem->bits, 0xaa, mem->size * mem->fwidth);

  mem->addr_root = 0x0;
  mem->name = name;
}

static void update_addr(vvp_memory_port_t addr);

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

void memory_init_nibble(vvp_memory_t mem, unsigned idx, unsigned char val)
{
  assert(idx < mem->size*mem->fwidth);
  mem->bits[idx] = val;
}

// Utilities

inline static
vvp_memory_bits_t get_word_ix(vvp_memory_t mem, unsigned idx)
{
  return mem->bits + idx*mem->fwidth;
}

inline static
vvp_memory_bits_t get_word(vvp_memory_t mem, int addr)
{
  assert(mem->a_idxs==1);
  unsigned waddr = addr - mem->a_idx[0].first;

  if (waddr >= mem->size)
    return 0x0;

  return get_word_ix(mem, waddr);
}

inline static
bool set_bit(vvp_memory_bits_t bits, int bit, unsigned char val)
{
  int ix =    bit/4;
  int ip = 2*(bit%4);
  bool r = ((bits[ix] >> ip) & 3) != val;
  bits[ix] = (bits[ix] &~ (3<<ip)) | ((val&3) << ip);
  return r;
}

inline static
unsigned char get_nibble(vvp_memory_bits_t bits, int bit)
{
  if (!bits)
    return 0xaa;
  int ix = bit/4;
  return bits[ix];
}

inline static
unsigned char get_bit(vvp_memory_bits_t bits, int bit)
{
  return (get_nibble(bits, bit) >> (2*(bit&3))) & 3;
}

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


// %set/mem

void memory_set(vvp_memory_t mem, unsigned idx, unsigned char val)
{
  if (idx/4 >= (mem->size * mem->fwidth))
    return;

  if (!set_bit(mem->bits, idx, val))
    return;

  unsigned widx = idx/(4*mem->fwidth);
  unsigned bidx = idx%(4*mem->fwidth);

  update_data_ports(mem, get_word_ix(mem, widx), bidx, val);

  // execute vpiMemory callbacks
  for (struct __vpiCallback*cur = mem->cb ;  cur ;  cur = cur->next) {
    cur->cb_data.time->type = vpiSimTime;
    cur->cb_data.index = widx; // assign the memory word index
    vpip_time_to_timestruct(cur->cb_data.time, schedule_simtime());
    assert(cur->cb_data.cb_rtn != 0);
    vpi_mode_flag = VPI_MODE_RWSYNC;
    (cur->cb_data.cb_rtn)(&cur->cb_data);
    vpi_mode_flag = VPI_MODE_NONE;
  }
}

// %load/mem

unsigned memory_get(vvp_memory_t mem, unsigned idx)
{
  if (idx/4 >= (mem->size * mem->fwidth))
    return 2;

  return get_bit(mem->bits, idx);
}

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

static void run_mem_assign(vvp_gen_event_t obj, unsigned char val)
{
  struct mem_assign_s *e = (struct mem_assign_s *) obj;
  memory_set(e->mem, e->idx, val);
  ma_free(e);
}

void schedule_memory(vvp_memory_t mem, unsigned idx,
		     unsigned char val, unsigned delay)
{
  struct mem_assign_s *e = ma_alloc();
  e->run = run_mem_assign;
  e->mem = mem;
  e->idx = idx;
  schedule_generic(e, val, delay, false);
}
