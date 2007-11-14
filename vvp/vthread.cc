/*
 * Copyright (c) 2001-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vthread.cc,v 1.166 2007/06/13 01:03:57 steve Exp $"
#endif

# include  "config.h"
# include  "vthread.h"
# include  "codes.h"
# include  "schedule.h"
# include  "ufunc.h"
# include  "event.h"
# include  "vpi_priv.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <limits.h>
# include  <string.h>
# include  <math.h>
# include  <assert.h>

# include  <iostream>
#include  <stdio.h>

/* This is the size of an unsigned long in bits. This is just a
   convenience macro. */
# define CPU_WORD_BITS (8*sizeof(unsigned long))
# define TOP_BIT (1UL << (CPU_WORD_BITS-1))

/*
 * This vhtread_s structure describes all there is to know about a
 * thread, including its program counter, all the private bits it
 * holds, and its place in other lists.
 *
 *
 * ** Notes On The Interactions of %fork/%join/%end:
 *
 * The %fork instruction creates a new thread and pushes that onto the
 * stack of children for the thread. This new thread, then, becomes
 * the new direct descendant of the thread. This new thread is
 * therefore also the first thread to be reaped when the parent does a
 * %join.
 *
 * It is a programming error for a thread that created threads to not
 * %join as many as it created before it %ends. The linear stack for
 * tracking thread relationships will create a mess otherwise. For
 * example, if A creates B then C, the stack is:
 *
 *       A --> C --> B
 *
 * If C then %forks X, the stack is:
 *
 *       A --> C --> X --> B
 *
 * If C %ends without a join, then the stack is:
 *
 *       A --> C(zombie) --> X --> B
 *
 * If A then executes 2 %joins, it will reap C and X (when it ends)
 * leaving B in purgatory. What's worse, A will block on the schedules
 * of X and C instead of C and B, possibly creating incorrect timing.
 *
 * The schedule_parent_on_end flag is used by threads to tell their
 * children that they are waiting for it to end. It is set by a %join
 * instruction if the child is not already done. The thread that
 * executes a %join instruction sets the flag in its child.
 *
 * The i_have_ended flag, on the other hand, is used by threads to
 * tell their parents that they are already dead. A thread that
 * executes %end will set its own i_have_ended flag and let its parent
 * reap it when the parent does the %join. If a thread has its
 * schedule_parent_on_end flag set already when it %ends, then it
 * reaps itself and simply schedules its parent. If a child has its
 * i_have_ended flag set when a thread executes %join, then it is free
 * to reap the child immediately.
 */

struct vthread_s {
	/* This is the program counter. */
      vvp_code_t pc;
	/* These hold the private thread bits. */
      vvp_vector4_t bits4;

	/* These are the word registers. */
      union {
	    int64_t  w_int;
	    uint64_t w_uint;
	    double   w_real;
      } words[16];

	/* My parent sets this when it wants me to wake it up. */
      unsigned schedule_parent_on_end :1;
      unsigned i_have_ended      :1;
      unsigned waiting_for_event :1;
      unsigned is_scheduled      :1;
      unsigned fork_count        :8;
	/* This points to the sole child of the thread. */
      struct vthread_s*child;
	/* This points to my parent, if I have one. */
      struct vthread_s*parent;
	/* This is used for keeping wait queues. */
      struct vthread_s*wait_next;
	/* These are used to keep the thread in a scope. */
      struct vthread_s*scope_next, *scope_prev;
};


static inline void thr_check_addr(struct vthread_s*thr, unsigned addr)
{
      if (thr->bits4.size() <= addr)
	    thr->bits4.resize(addr + CPU_WORD_BITS);
}

static inline vvp_bit4_t thr_get_bit(struct vthread_s*thr, unsigned addr)
{
      assert(addr < thr->bits4.size());
      return thr->bits4.value(addr);
}

static inline void thr_put_bit(struct vthread_s*thr,
			       unsigned addr, vvp_bit4_t val)
{
      thr_check_addr(thr, addr);
      thr->bits4.set_bit(addr, val);
}

// REMOVE ME
static inline void thr_clr_bit_(struct vthread_s*thr, unsigned addr)
{
      thr->bits4.set_bit(addr, BIT4_0);
}

vvp_bit4_t vthread_get_bit(struct vthread_s*thr, unsigned addr)
{
      return thr_get_bit(thr, addr);
}

void vthread_put_bit(struct vthread_s*thr, unsigned addr, vvp_bit4_t bit)
{
      thr_put_bit(thr, addr, bit);
}

double vthread_get_real(struct vthread_s*thr, unsigned addr)
{
      return thr->words[addr].w_real;
}

void vthread_put_real(struct vthread_s*thr, unsigned addr, double val)
{
      thr->words[addr].w_real = val;
}

static unsigned long* vector_to_array(struct vthread_s*thr,
				      unsigned addr, unsigned wid)
{
      unsigned awid = (wid + CPU_WORD_BITS - 1) / (CPU_WORD_BITS);


      if (addr == 0) {
	    unsigned long*val = new unsigned long[awid];
	    for (unsigned idx = 0 ;  idx < awid ;  idx += 1)
		  val[idx] = 0;
	    return val;
      }
      if (addr == 1) {
	    unsigned long*val = new unsigned long[awid];
	    for (unsigned idx = 0 ;  idx < awid ;  idx += 1)
		  val[idx] = -1UL;
	    return val;
      }

      if (addr < 4)
	    return 0;

      return thr->bits4.subarray(addr, wid);
}

/*
 * This function gets from the thread a vector of bits starting from
 * the addressed location and for the specified width.
 */
static vvp_vector4_t vthread_bits_to_vector(struct vthread_s*thr,
					    unsigned bit, unsigned wid)
{
      	/* Make a vector of the desired width. */

      if (bit >= 4) {
	    return vvp_vector4_t(thr->bits4, bit, wid);

      } else {
	    vvp_vector4_t value(wid);
	    vvp_bit4_t bit_val = (vvp_bit4_t)bit;
	    for (unsigned idx = 0; idx < wid; idx +=1) {
		  value.set_bit(idx, bit_val);
	    }
	    return value;
      }
}


/*
 * Create a new thread with the given start address.
 */
vthread_t vthread_new(vvp_code_t pc, struct __vpiScope*scope)
{
      vthread_t thr = new struct vthread_s;
      thr->pc     = pc;
      thr->bits4  = vvp_vector4_t(32);
      thr->child  = 0;
      thr->parent = 0;
      thr->wait_next = 0;

	/* If the target scope never held a thread, then create a
	   header cell for it. This is a stub to make circular lists
	   easier to work with. */
      if (scope->threads == 0) {
	    scope->threads = new struct vthread_s;
	    scope->threads->pc = codespace_null();
	    scope->threads->bits4  = vvp_vector4_t();
	    scope->threads->child  = 0;
	    scope->threads->parent = 0;
	    scope->threads->scope_prev = scope->threads;
	    scope->threads->scope_next = scope->threads;
      }

      { vthread_t tmp = scope->threads;
        thr->scope_next = tmp->scope_next;
	thr->scope_prev = tmp;
	thr->scope_next->scope_prev = thr;
	thr->scope_prev->scope_next = thr;
      }

      thr->schedule_parent_on_end = 0;
      thr->is_scheduled = 0;
      thr->i_have_ended = 0;
      thr->waiting_for_event = 0;
      thr->is_scheduled = 0;
      thr->fork_count   = 0;

      thr_put_bit(thr, 0, BIT4_0);
      thr_put_bit(thr, 1, BIT4_1);
      thr_put_bit(thr, 2, BIT4_X);
      thr_put_bit(thr, 3, BIT4_Z);

      return thr;
}

/*
 * Reaping pulls the thread out of the stack of threads. If I have a
 * child, then hand it over to my parent.
 */
static void vthread_reap(vthread_t thr)
{
      thr->bits4 = vvp_vector4_t();

      if (thr->child) {
	    assert(thr->child->parent == thr);
	    thr->child->parent = thr->parent;
      }
      if (thr->parent) {
	    assert(thr->parent->child == thr);
	    thr->parent->child = thr->child;
      }

      thr->child = 0;
      thr->parent = 0;

      thr->scope_next->scope_prev = thr->scope_prev;
      thr->scope_prev->scope_next = thr->scope_next;

      thr->pc = codespace_null();

	/* If this thread is not scheduled, then is it safe to delete
	   it now. Otherwise, let the schedule event (which will
	   execute the thread at of_ZOMBIE) delete the object. */
      if ((thr->is_scheduled == 0) && (thr->waiting_for_event == 0)) {
	    assert(thr->fork_count == 0);
	    assert(thr->wait_next == 0);
	    delete thr;
      }
}

void vthread_mark_scheduled(vthread_t thr)
{
      while (thr != 0) {
	    assert(thr->is_scheduled == 0);
	    thr->is_scheduled = 1;
	    thr = thr->wait_next;
      }
}

/*
 * This function runs each thread by fetching an instruction,
 * incrementing the PC, and executing the instruction. The thread may
 * be the head of a list, so each thread is run so far as possible.
 */
void vthread_run(vthread_t thr)
{
      while (thr != 0) {
	    vthread_t tmp = thr->wait_next;
	    thr->wait_next = 0;

	    assert(thr->is_scheduled);
	    thr->is_scheduled = 0;

	    for (;;) {
		  vvp_code_t cp = thr->pc;
		  thr->pc += 1;

		    /* Run the opcode implementation. If the execution of
		       the opcode returns false, then the thread is meant to
		       be paused, so break out of the loop. */
		  bool rc = (cp->opcode)(thr, cp);
		  if (rc == false)
			break;
	    }

	    thr = tmp;
      }
}

/*
 * Unlink a ptr object from the driver. The input is the driver in the
 * form of a vvp_net_t pointer. The .out member of that object is the
 * driver. The dst_ptr argument is the receiver pin to be located and
 * removed from the fan-out list.
 */
static void unlink_from_driver(vvp_net_t*src, vvp_net_ptr_t dst_ptr)
{
      vvp_net_t*net = dst_ptr.ptr();
      unsigned net_port = dst_ptr.port();

      if (src->out == dst_ptr) {
	      /* If the drive fan-out list starts with this pointer,
		 then the unlink is easy. Pull the list forward. */
	    src->out = net->port[net_port];
      } else {
	      /* Scan the linked list, looking for the net_ptr_t
		 pointer *before* the one we wish to remove. */
	    vvp_net_ptr_t cur = src->out;
	    assert(!cur.nil());
	    vvp_net_t*cur_net = cur.ptr();
	    unsigned cur_port = cur.port();
	    while (cur_net->port[cur_port] != dst_ptr) {
		  cur = cur_net->port[cur_port];
		  assert(!cur.nil());
		  cur_net = cur.ptr();
		  cur_port = cur.port();
	    }
	      /* Unlink. */
	    cur_net->port[cur_port] = net->port[net_port];
      }

      net->port[net_port] = vvp_net_ptr_t(0,0);
}

/*
 * The CHUNK_LINK instruction is a specla next pointer for linking
 * chunks of code space. It's like a simplified %jmp.
 */
bool of_CHUNK_LINK(vthread_t thr, vvp_code_t code)
{
      assert(code->cptr);
      thr->pc = code->cptr;
      return true;
}

/*
 * This is called by an event functor to wake up all the threads on
 * its list. I in fact created that list in the %wait instruction, and
 * I also am certain that the waiting_for_event flag is set.
 */
void vthread_schedule_list(vthread_t thr)
{
      for (vthread_t cur = thr ;  cur ;  cur = cur->wait_next) {
	    assert(cur->waiting_for_event);
	    cur->waiting_for_event = 0;
      }

      schedule_vthread(thr, 0);
}


bool of_AND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    thr_put_bit(thr, idx1, lb & rb);

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}


bool of_ADD(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned long*lva = vector_to_array(thr, cp->bit_idx[0], cp->number);
      unsigned long*lvb = vector_to_array(thr, cp->bit_idx[1], cp->number);
      if (lva == 0 || lvb == 0)
	    goto x_out;


      unsigned long carry;
      carry = 0;
      for (unsigned idx = 0 ;  (idx*CPU_WORD_BITS) < cp->number ;  idx += 1) {

	    unsigned long tmp = lvb[idx] + carry;
	    unsigned long sum = lva[idx] + tmp;
	    carry = 0;
	    if (tmp < lvb[idx])
		  carry = 1;
	    if (sum < tmp)
		  carry = 1;
	    if (sum < lva[idx])
		  carry = 1;
	    lva[idx] = sum;
      }

	/* We know from the vector_to_array that the address is valid
	   in the thr->bitr4 vector, so just do the set bit. */

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned bit = lva[idx/CPU_WORD_BITS] >> (idx % CPU_WORD_BITS);
	    thr->bits4.set_bit(cp->bit_idx[0]+idx, (bit&1) ? BIT4_1 : BIT4_0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;
      delete[]lvb;

      vvp_vector4_t tmp(cp->number, BIT4_X);
      thr->bits4.set_vec(cp->bit_idx[0], tmp);

      return true;
}

bool of_ADD_WR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = l + r;
      return true;
}

/*
 * This is %addi, add-immediate. The first value is a vector, the
 * second value is the immediate value in the bin_idx[1] position. The
 * immediate value can be up to 16 bits, which are then padded to the
 * width of the vector with zero.
 */
bool of_ADDI(vthread_t thr, vvp_code_t cp)
{
	// Collect arguments
      unsigned bit_addr       = cp->bit_idx[0];
      unsigned long imm_value = cp->bit_idx[1];
      unsigned bit_width      = cp->number;

      assert(bit_addr >= 4);

      unsigned word_count = (bit_width+CPU_WORD_BITS-1)/CPU_WORD_BITS;

      unsigned long*lva = vector_to_array(thr, bit_addr, bit_width);
      unsigned long*lvb = 0;
      if (lva == 0)
	    goto x_out;

      lvb = new unsigned long[word_count];

      lvb[0] = imm_value;
      for (unsigned idx = 1 ;  idx < word_count ;  idx += 1)
	    lvb[idx] = 0;

      unsigned long carry;
      carry = 0;
      for (unsigned idx = 0 ;  (idx*CPU_WORD_BITS) < bit_width ;  idx += 1) {

	    unsigned long tmp = lvb[idx] + carry;
	    unsigned long sum = lva[idx] + tmp;
	    carry = 0;
	    if (tmp < lvb[idx])
		  carry = 1;
	    if (sum < tmp)
		  carry = 1;
	    if (sum < lva[idx])
		  carry = 1;
	    lva[idx] = sum;
      }

      thr_check_addr(thr, bit_addr + bit_width - 1);
      for (unsigned idx = 0 ;  idx < bit_width ;  idx += 1) {
	    unsigned long bit = lva[idx/CPU_WORD_BITS] >> (idx%CPU_WORD_BITS);
	    thr->bits4.set_bit(bit_addr+idx, (bit&1UL) ? BIT4_1:BIT4_0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;

      vvp_vector4_t tmp (bit_width, BIT4_X);
      thr->bits4.set_vec(bit_addr, tmp);

      return true;
}

/* %assign/av <array>, <delay>, <bit>
 * This generates an assignment event to an array. Index register 0
 * contains the width of the vector (and the word) and index register
 * 3 contains the canonical address of the word in memory.
 */
bool of_ASSIGN_AV(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      unsigned off = thr->words[1].w_int;
      unsigned adr = thr->words[3].w_int;

      assert(wid > 0);

      unsigned delay = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      schedule_assign_array_word(cp->array, adr, off, value, delay);
      return true;
}

/*
 * This is %assign/v0 <label>, <delay>, <bit>
 * Index register 0 contains a vector width.
 */
bool of_ASSIGN_V0(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      assert(wid > 0);
      unsigned delay = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
      schedule_assign_vector(ptr, value, delay);

      return true;
}

/*
 * This is %assign/v0/d <label>, <delay_idx>, <bit>
 * Index register 0 contains a vector width, and the named index
 * register contains the delay.
 */
bool of_ASSIGN_V0D(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      assert(wid > 0);

      unsigned long delay = thr->words[cp->bit_idx[0]].w_int;
      unsigned bit = cp->bit_idx[1];

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
      schedule_assign_vector(ptr, value, delay);

      return true;
}

/*
 * This is %assign/v0/x1 <label>, <delay>, <bit>
 * Index register 0 contains a vector part width.
 * Index register 1 contains the offset into the destination vector.
 */
bool of_ASSIGN_V0X1(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      unsigned off = thr->words[1].w_int;
      unsigned delay = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];

      vvp_fun_signal_vec*sig
	    = reinterpret_cast<vvp_fun_signal_vec*> (cp->net->fun);
      assert(sig);
      assert(wid > 0);

      if (off >= sig->size())
	    return true;

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
      schedule_assign_vector(ptr, off, sig->size(), value, delay);

      return true;
}

/*
 * This is %assign/v0/x1 <label>, <delayx>, <bit>
 * Index register 0 contains a vector part width.
 * Index register 1 contains the offset into the destination vector.
 */
bool of_ASSIGN_V0X1D(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      unsigned off = thr->words[1].w_int;
      unsigned delay = thr->words[cp->bit_idx[0]].w_int;
      unsigned bit = cp->bit_idx[1];

      vvp_fun_signal_vec*sig
	    = reinterpret_cast<vvp_fun_signal_vec*> (cp->net->fun);
      assert(sig);
      assert(wid > 0);

      if (off >= sig->size())
	    return true;

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
      schedule_assign_vector(ptr, off, sig->size(), value, delay);

      return true;
}

/*
 * This is %assign/wr <vpi-label>, <delay>, <index>
 *
 * This assigns (after a delay) a value to a real variable. Use the
 * vpi_put_value function to do the assign, with the delay written
 * into the vpiInertialDelay carrying the desired delay.
 */
bool of_ASSIGN_WR(vthread_t thr, vvp_code_t cp)
{
      unsigned delay = cp->bit_idx[0];
      unsigned index = cp->bit_idx[1];
      s_vpi_time del;

      del.type = vpiSimTime;
      vpip_time_to_timestruct(&del, delay);

      struct __vpiHandle*tmp = cp->handle;

      t_vpi_value val;
      val.format = vpiRealVal;
      val.value.real = thr->words[index].w_real;
      vpi_put_value(tmp, &val, &del, vpiInertialDelay);

      return true;
}

bool of_ASSIGN_X0(vthread_t thr, vvp_code_t cp)
{
#if 0
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[1]);
      vvp_ipoint_t itmp = ipoint_index(cp->iptr, thr->words[0].w_int);
      schedule_assign(itmp, bit_val, cp->bit_idx[0]);
#else
      fprintf(stderr, "XXXX forgot how to implement %%assign/x0\n");
#endif
      return true;
}

/* %assign/mv <memory>, <delay>, <bit>
 * This generates an assignment event to a memory. Index register 0
 * contains the width of the vector (and the word) and index register
 * 3 contains the canonical address of the word in memory.
 */
bool of_ASSIGN_MV(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      unsigned off = thr->words[1].w_int;
      unsigned adr = thr->words[3].w_int;

      assert(wid > 0);

      unsigned delay = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      schedule_assign_memory_word(cp->mem, adr, off, value, delay);
      return true;
}

bool of_BLEND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if (lb != rb)
		  thr_put_bit(thr, idx1, BIT4_X);

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

bool of_BREAKPOINT(vthread_t thr, vvp_code_t cp)
{
      return true;
}

/*
 * the %cassign/link instruction connects a source node to a
 * destination node. The destination node must be a signal, as it is
 * marked with the source of the cassign so that it may later be
 * unlinked without specifically knowing the source that this
 * instruction used.
 */
bool of_CASSIGN_LINK(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*dst = cp->net;
      vvp_net_t*src = cp->net2;

      vvp_fun_signal_base*sig
	    = reinterpret_cast<vvp_fun_signal_base*>(dst->fun);
      assert(sig);

	/* Detect the special case that we are already continuous
	   assigning the source onto the destination. */
      if (sig->cassign_link == src)
	    return true;

	/* If there is an existing cassign driving this node, then
	   unlink it. We can have only 1 cassign at a time. */
      if (sig->cassign_link != 0) {
	    vvp_net_ptr_t tmp (dst,1);
	    unlink_from_driver(sig->cassign_link, tmp);
      }

      sig->cassign_link = src;

	/* Link the output of the src to the port[1] (the cassign
	   port) of the destination. */
      vvp_net_ptr_t dst_ptr (dst, 1);
      dst->port[1] = src->out;
      src->out = dst_ptr;

      return true;
}

/*
 * the %cassign/v instruction invokes a continuous assign of a
 * constant value to a signal. The instruction arguments are:
 *
 *     %cassign/v <net>, <base>, <wid> ;
 *
 * Where the <net> is the net label assembled into a vvp_net pointer,
 * and the <base> and <wid> are stashed in the bit_idx array.
 *
 * This instruction writes vvp_vector4_t values to port-1 of the
 * target signal.
 */
bool of_CASSIGN_V(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net  = cp->net;
      unsigned  base = cp->bit_idx[0];
      unsigned  wid  = cp->bit_idx[1];

	/* Collect the thread bits into a vector4 item. */
      vvp_vector4_t value = vthread_bits_to_vector(thr, base, wid);

	/* set the value into port 1 of the destination. */
      vvp_net_ptr_t ptr (net, 1);
      vvp_send_vec4(ptr, value);

      return true;
}

bool of_CMPS(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq  = BIT4_1;
      vvp_bit4_t eeq = BIT4_1;
      vvp_bit4_t lt  = BIT4_0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      const unsigned end1 = (idx1 < 4)? idx1 : idx1 + cp->number - 1;
      const unsigned end2 = (idx2 < 4)? idx2 : idx2 + cp->number - 1;

      if (end1 > end2)
	    thr_check_addr(thr, end1);
      else
	    thr_check_addr(thr, end2);

      const vvp_bit4_t sig1 = thr_get_bit(thr, end1);
      const vvp_bit4_t sig2 = thr_get_bit(thr, end2);

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = thr_get_bit(thr, idx2);

	    if (lv > rv) {
		  lt  = BIT4_0;
		  eeq = BIT4_0;
	    } else if (lv < rv) {
		  lt  = BIT4_1;
		  eeq = BIT4_0;
	    }
	    if (eq != BIT4_X) {
		  if ((lv == BIT4_0) && (rv != BIT4_0))
			eq = BIT4_0;
		  if ((lv == BIT4_1) && (rv != BIT4_1))
			eq = BIT4_0;
		  if (bit4_is_xz(lv) || bit4_is_xz(rv))
			eq = BIT4_X;
	    }

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;
      }

      if (eq == BIT4_X)
	    lt = BIT4_X;
      else if ((sig1 == BIT4_1) && (sig2 == BIT4_0))
	    lt = BIT4_1;
      else if ((sig1 == BIT4_0) && (sig2 == BIT4_1))
	    lt = BIT4_0;

	/* Correct the lt bit to account for the sign of the parameters. */
      if (lt != BIT4_X) {
	      /* If the first is negative and the last positive, then
		 a < b for certain. */
	    if ((sig1 == BIT4_1) && (sig2 == BIT4_0))
		  lt = BIT4_1;

	      /* If the first is positive and the last negative, then
		 a > b for certain. */
	    if ((sig1 == BIT4_0) && (sig2 == BIT4_1))
		  lt = BIT4_0;
      }

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPIS(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq  = BIT4_1;
      vvp_bit4_t eeq = BIT4_1;
      vvp_bit4_t lt  = BIT4_0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned imm  = cp->bit_idx[1];

      const unsigned end1 = (idx1 < 4)? idx1 : idx1 + cp->number - 1;
      thr_check_addr(thr, end1);
      const vvp_bit4_t sig1 = thr_get_bit(thr, end1);

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = (imm & 1)? BIT4_1 : BIT4_0;
	    imm >>= 1;

	    if (lv > rv) {
		  lt = BIT4_0;
		  eeq = BIT4_0;
	    } else if (lv < rv) {
		  lt = BIT4_1;
		  eeq = BIT4_0;
	    }
	    if (eq != BIT4_X) {
		  if ((lv == BIT4_0) && (rv != BIT4_0))
			eq = BIT4_0;
		  if ((lv == BIT4_1) && (rv != BIT4_1))
			eq = BIT4_0;
		  if (bit4_is_xz(lv) || bit4_is_xz(rv))
			eq = BIT4_X;
	    }

	    if (idx1 >= 4) idx1 += 1;
      }

      if (eq == BIT4_X)
	    lt = BIT4_X;
      else if (sig1 == BIT4_1)
	    lt = BIT4_1;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPIU(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq  = BIT4_1;
      vvp_bit4_t eeq = BIT4_1;
      vvp_bit4_t lt  = BIT4_0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned imm  = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = (imm & 1)? BIT4_1 : BIT4_0;
	    imm >>= 1;

	    if (lv > rv) {
		  lt = BIT4_0;
		  eeq = BIT4_0;
	    } else if (lv < rv) {
		  lt = BIT4_1;
		  eeq = BIT4_0;
	    }
	    if (eq != BIT4_X) {
		  if ((lv == BIT4_0) && (rv != BIT4_0))
			eq = BIT4_0;
		  if ((lv == BIT4_1) && (rv != BIT4_1))
			eq = BIT4_0;
		  if (bit4_is_xz(lv) || bit4_is_xz(rv))
			eq = BIT4_X;
	    }

	    if (idx1 >= 4) idx1 += 1;
      }

      if (eq == BIT4_X)
	    lt = BIT4_X;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPU(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq = BIT4_1;
      vvp_bit4_t eeq = BIT4_1;
      vvp_bit4_t lt = BIT4_0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = thr_get_bit(thr, idx2);

	    if (lv > rv) {
		  lt  = BIT4_0;
		  eeq = BIT4_0;
	    } else if (lv < rv) {
		  lt  = BIT4_1;
		  eeq = BIT4_0;
	    }
	    if (eq != BIT4_X) {
		  if ((lv == BIT4_0) && (rv != BIT4_0))
			eq = BIT4_0;
		  if ((lv == BIT4_1) && (rv != BIT4_1))
			eq = BIT4_0;
		  if (bit4_is_xz(lv) || bit4_is_xz(rv))
			eq = BIT4_X;
	    }

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;

      }

      if (eq == BIT4_X)
	    lt = BIT4_X;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPX(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq = BIT4_1;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = thr_get_bit(thr, idx2);

	    if ((lv != rv) && !bit4_is_xz(lv) && !bit4_is_xz(rv)) {
		  eq = BIT4_0;
		  break;
	    }

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;
      }

      thr_put_bit(thr, 4, eq);

      return true;
}

bool of_CMPWR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;

      vvp_bit4_t eq = (l == r)? BIT4_1 : BIT4_0;
      vvp_bit4_t lt = (l <  r)? BIT4_1 : BIT4_0;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);

      return true;
}

bool of_CMPWS(vthread_t thr, vvp_code_t cp)
{
      int64_t l = thr->words[cp->bit_idx[0]].w_int;
      int64_t r = thr->words[cp->bit_idx[1]].w_int;

      vvp_bit4_t eq = (l == r)? BIT4_1 : BIT4_0;
      vvp_bit4_t lt = (l <  r)? BIT4_1 : BIT4_0;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);

      return true;
}

bool of_CMPWU(vthread_t thr, vvp_code_t cp)
{
      uint64_t l = thr->words[cp->bit_idx[0]].w_uint;
      uint64_t r = thr->words[cp->bit_idx[1]].w_uint;

      vvp_bit4_t eq = (l == r)? BIT4_1 : BIT4_0;
      vvp_bit4_t lt = (l <  r)? BIT4_1 : BIT4_0;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);

      return true;
}

bool of_CMPZ(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq = BIT4_1;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = thr_get_bit(thr, idx2);

	    if ((lv != BIT4_Z) && (rv != BIT4_Z) && (lv != rv)) {
		  eq = BIT4_0;
		  break;
	    }

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;
      }

      thr_put_bit(thr, 4, eq);

      return true;
}

bool of_CVT_IR(vthread_t thr, vvp_code_t cp)
{
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_int = lround(r);

      return true;
}

bool of_CVT_RI(vthread_t thr, vvp_code_t cp)
{
      long r = thr->words[cp->bit_idx[1]].w_int;
      thr->words[cp->bit_idx[0]].w_real = (double)(r);

      return true;
}

bool of_CVT_VR(vthread_t thr, vvp_code_t cp)
{
      double r = thr->words[cp->bit_idx[1]].w_real;
      long rl = lround(r);
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    thr_put_bit(thr, base+idx, (rl&1)? BIT4_1 : BIT4_0);
	    rl >>= 1;
      }

      return true;
}

/*
 * This implements the %deassign instruction. All we do is write a
 * long(1) to port-3 of the addressed net. This turns off an active
 * continuous assign activated by %cassign/v
 *
 * FIXME: This does not remove a linked %cassign/link. It should.
 */
bool of_DEASSIGN(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;

      vvp_net_ptr_t ptr (net, 3);
      vvp_send_long(ptr, 1);

      return true;
}

/*
 * The delay takes two 32bit numbers to make up a 64bit time.
 *
 *   %delay <low>, <hig>
 */
bool of_DELAY(vthread_t thr, vvp_code_t cp)
{
      vvp_time64_t low = cp->bit_idx[0];
      vvp_time64_t hig = cp->bit_idx[1];

      vvp_time64_t res = 32;
      res = hig << res;
      res += low;

      schedule_vthread(thr, res);
      return false;
}

bool of_DELAYX(vthread_t thr, vvp_code_t cp)
{
      unsigned long delay;

      assert(cp->number < 4);
      delay = thr->words[cp->number].w_int;
      schedule_vthread(thr, delay);
      return false;
}

static bool do_disable(vthread_t thr, vthread_t match)
{
      bool flag = false;

	/* Pull the target thread out of its scope. */
      thr->scope_next->scope_prev = thr->scope_prev;
      thr->scope_prev->scope_next = thr->scope_next;

	/* Turn the thread off by setting is program counter to
	   zero and setting an OFF bit. */
      thr->pc = codespace_null();
      thr->i_have_ended = 1;

	/* Turn off all the children of the thread. Simulate a %join
	   for as many times as needed to clear the results of all the
	   %forks that this thread has done. */
      while (thr->fork_count > 0) {

	    vthread_t tmp = thr->child;
	    assert(tmp);
	    assert(tmp->parent == thr);
	    tmp->schedule_parent_on_end = 0;
	    if (do_disable(tmp, match))
		  flag = true;

	    thr->fork_count -= 1;

	    vthread_reap(tmp);
      }


      if (thr->schedule_parent_on_end) {
	      /* If a parent is waiting in a %join, wake it up. */
	    assert(thr->parent);
	    assert(thr->parent->fork_count > 0);

	    thr->parent->fork_count -= 1;
	    schedule_vthread(thr->parent, 0, true);
	    vthread_reap(thr);

      } else if (thr->parent) {
	      /* If the parent is yet to %join me, let its %join
		 do the reaping. */
	      //assert(tmp->is_scheduled == 0);

      } else {
	      /* No parent at all. Goodbye. */
	    vthread_reap(thr);
      }

      return flag || (thr == match);
}

/*
 * Implement the %disable instruction by scanning the target scope for
 * all the target threads. Kill the target threads and wake up a
 * parent that is attempting a %join.
 */
bool of_DISABLE(vthread_t thr, vvp_code_t cp)
{
      struct __vpiScope*scope = (struct __vpiScope*)cp->handle;
      if (scope->threads == 0)
	    return true;

      struct vthread_s*head = scope->threads;

      bool disabled_myself_flag = false;

      while (head->scope_next != head) {
	    vthread_t tmp = head->scope_next;

	      /* If I am disabling myself, that remember that fact so
		 that I can finish this statement differently. */
	    if (tmp == thr)
		  disabled_myself_flag = true;


	    if (do_disable(tmp, thr))
		  disabled_myself_flag = true;
      }

      return ! disabled_myself_flag;
}

static void divide_bits(unsigned len, unsigned char*lbits,
			const unsigned char*rbits)
{
      unsigned char *a, *b, *z, *t;
      a = new unsigned char[len+1];
      b = new unsigned char[len+1];
      z = new unsigned char[len+1];
      t = new unsigned char[len+1];

      unsigned char carry;
      unsigned char temp;

      int mxa = -1, mxz = -1;
      int i;
      int current, copylen;


      for (unsigned idx = 0 ;  idx < len ;  idx += 1) {
	    unsigned lb = lbits[idx];
	    unsigned rb = rbits[idx];

	    z[idx]=lb;
	    a[idx]=1-rb;	// for 2s complement add..

      }
      z[len]=0;
      a[len]=1;

      for(i=0;i<(int)len+1;i++) {
	    b[i]=0;
      }

      for(i=len-1;i>=0;i--) {
	    if(!a[i]) {
		  mxa=i;
		  break;
	    }
      }

      for(i=len-1;i>=0;i--) {
	    if(z[i]) {
		  mxz=i;
		  break;
	    }
      }

      if((mxa>mxz)||(mxa==-1)) {
	    if(mxa==-1) {
		  fprintf(stderr, "Division By Zero error, exiting.\n");
		  exit(255);
	    }

	    goto tally;
      }

      copylen = mxa + 2;
      current = mxz - mxa;

      while(current > -1) {
	    carry = 1;
	    for(i=0;i<copylen;i++) {
		  temp = z[i+current] + a[i] + carry;
		  t[i] = (temp&1);
		  carry = (temp>>1);
	    }

	    if(carry) {
		  for(i=0;i<copylen;i++) {
			z[i+current] = t[i];
		  }
		  b[current] = 1;
	    }

	    current--;
      }

 tally:
      for (unsigned idx = 0 ;  idx < len ;  idx += 1) {
	      // n.b., z[] has the remainder...
	    lbits[idx] = b[idx];
      }

      delete []t;
      delete []z;
      delete []b;
      delete []a;
}

bool of_DIV(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if(cp->number <= 8*sizeof(unsigned long)) {
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned idx2 = cp->bit_idx[1];
	    unsigned long lv = 0, rv = 0;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  vvp_bit4_t lb = thr_get_bit(thr, idx1);
		  vvp_bit4_t rb = thr_get_bit(thr, idx2);

		  if (bit4_is_xz(lb) || bit4_is_xz(rb))
			goto x_out;

		  lv |= (unsigned long) lb << idx;
		  rv |= (unsigned long) rb << idx;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	    if (rv == BIT4_0)
		  goto x_out;

	    lv /= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? BIT4_1 : BIT4_0);
		  lv >>= 1;
	    }

	    return true;

      } else {

	      /* Make a string of the bits of the numbers to be
		 divided. Then divide them, and write the results into
		 the thread. */
	    unsigned char*lbits = new unsigned char[cp->number];
	    unsigned char*rbits = new unsigned char[cp->number];
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned idx2 = cp->bit_idx[1];
	    bool rval_is_zero = true;
	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  lbits[idx] = thr_get_bit(thr, idx1);
		  rbits[idx] = thr_get_bit(thr, idx2);
		  if ((lbits[idx] | rbits[idx]) > 1) {
			delete[]lbits;
			delete[]rbits;
			goto x_out;
		  }

		  if (rbits[idx] != 0)
			rval_is_zero = false;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	      /* Notice the special case of divide by 0. */
	    if (rval_is_zero) {
		  delete[]lbits;
		  delete[]rbits;
		  goto x_out;
	    }

	    divide_bits(cp->number, lbits, rbits);

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, lbits[idx]?BIT4_1:BIT4_0);
	    }

	    delete[]lbits;
	    delete[]rbits;
	    return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

static void negate_bits(unsigned len, unsigned char*bits)
{
      unsigned char carry = 1;
      for (unsigned idx = 0 ;  idx < len ;  idx += 1) {
	    carry += bits[idx]? 0 : 1;
	    bits[idx] = carry & 1;
	    carry >>= 1;
      }
}

bool of_DIV_S(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if(cp->number <= 8*sizeof(long)) {
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned idx2 = cp->bit_idx[1];
	    long lv = 0, rv = 0;

	    unsigned lb = 0;
	    unsigned rb = 0;
	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  lb = thr_get_bit(thr, idx1);
		  rb = thr_get_bit(thr, idx2);

		  if ((lb | rb) & 2)
			goto x_out;

		  lv |= (long)lb << idx;
		  rv |= (long)rb << idx;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	      /* Extend the sign to fill the native long. */
	    for (unsigned idx = cp->number; idx < (8*sizeof lv); idx += 1) {
		  lv |= (long)lb << idx;
		  rv |= (long)rb << idx;
	    }

	    if (rv == 0)
		  goto x_out;

	    lv /= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1)?BIT4_1:BIT4_0);
		  lv >>= 1;
	    }

      } else {
	    unsigned char*lbits = new unsigned char[cp->number];
	    unsigned char*rbits = new unsigned char[cp->number];
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned idx2 = cp->bit_idx[1];
	    bool rval_is_zero = true;
	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  lbits[idx] = thr_get_bit(thr, idx1);
		  rbits[idx] = thr_get_bit(thr, idx2);
		  if ((lbits[idx] | rbits[idx]) > 1) {
			delete[]lbits;
			delete[]rbits;
			goto x_out;
		  }

		  if (rbits[idx] != 0)
			rval_is_zero = false;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	      /* Notice the special case of divide by 0. */
	    if (rval_is_zero) {
		  delete[]lbits;
		  delete[]rbits;
		  goto x_out;
	    }

	      /* Signed division is unsigned division on the absolute
		 values of the operands, then corrected for the number
		 of signs. */
	    unsigned sign_flag = 0;
	    if (lbits[cp->number-1]) {
		  sign_flag += 1;
		  negate_bits(cp->number, lbits);
	    }
	    if (rbits[cp->number-1]) {
		  sign_flag += 1;
		  negate_bits(cp->number, rbits);
	    }

	    divide_bits(cp->number, lbits, rbits);

	    if (sign_flag & 1) {
		  negate_bits(cp->number, lbits);
	    }

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, lbits[idx]?BIT4_1:BIT4_0);
	    }

	    delete[]lbits;
	    delete[]rbits;
      }

      return true;

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

bool of_DIV_WR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = l / r;

      return true;
}

/*
 * This terminates the current thread. If there is a parent who is
 * waiting for me to die, then I schedule it. At any rate, I mark
 * myself as a zombie by setting my pc to 0.
 *
 * It is possible for this thread to have children at this %end. This
 * means that my child is really my sibling created by my parent, and
 * my parent will do the proper %joins in due course. For example:
 *
 *     %fork child_1, test;
 *     %fork child_2, test;
 *     ... parent code ...
 *     %join;
 *     %join;
 *     %end;
 *
 *   child_1 ;
 *     %end;
 *   child_2 ;
 *     %end;
 *
 * In this example, the main thread creates threads child_1 and
 * child_2. It is possible that this thread is child_2, so there is a
 * parent pointer and a child pointer, even though I did no
 * %forks or %joins. This means that I have a ->child pointer and a
 * ->parent pointer.
 *
 * If the main thread has executed the first %join, then it is waiting
 * for me, and I will be reaped right away.
 *
 * If the main thread has not executed a %join yet, then this thread
 * becomes a zombie. The main thread executes its %join eventually,
 * reaping me at that time.
 *
 * It does not matter the order that child_1 and child_2 threads call
 * %end -- child_2 will be reaped by the first %join, and child_1 will
 * be reaped by the second %join.
 */
bool of_END(vthread_t thr, vvp_code_t)
{
      assert(! thr->waiting_for_event);
      assert( thr->fork_count == 0 );
      thr->i_have_ended = 1;
      thr->pc = codespace_null();

	/* If I have a parent who is waiting for me, then mark that I
	   have ended, and schedule that parent. Also, finish the
	   %join for the parent. */
      if (thr->schedule_parent_on_end) {
	    assert(thr->parent);
	    assert(thr->parent->fork_count > 0);

	    thr->parent->fork_count -= 1;
	    schedule_vthread(thr->parent, 0, true);
	    vthread_reap(thr);
	    return false;
      }

	/* If I have no parents, then no one can %join me and there is
	   no reason to stick around. This can happen, for example if
	   I am an ``initial'' thread.

	   If I have children at this point, then I must have been the
	   main thread (there is no other parent) and an error (not
	   enough %joins) has been detected. */
      if (thr->parent == 0) {
	    assert(thr->child == 0);
	    vthread_reap(thr);
	    return false;
      }

	/* If I make it this far, then I have a parent who may wish
	   to %join me. Remain a zombie so that it can. */

      return false;
}

static void unlink_force(vvp_net_t*net)
{
      vvp_fun_signal_base*sig
	    = reinterpret_cast<vvp_fun_signal_base*>(net->fun);
	/* This node must be a signal... */
      assert(sig);
	/* This signal is being forced. */
      assert(sig->force_link);

      vvp_net_t*src = sig->force_link;
      sig->force_link = 0;

	/* We are looking for this pointer. */
      vvp_net_ptr_t net_ptr (net, 2);

	/* If net is first in the fan-out list, then simply pull it
	   from the front. */
      if (src->out == net_ptr) {
	    src->out = net->port[2];
	    net->port[2] = vvp_net_ptr_t();
	    return;
      }

	/* Look for the pointer in the fan-out chain */
      vvp_net_ptr_t cur_ptr = src->out;
      assert(!cur_ptr.nil());
      while (cur_ptr.ptr()->port[cur_ptr.port()] != net_ptr) {
	    cur_ptr = cur_ptr.ptr()->port[cur_ptr.port()];
	    assert( !cur_ptr.nil() );
      }

	/* Remove as if from a singly-linked list. */
      cur_ptr.ptr()->port[cur_ptr.port()] = net->port[2];
      net->port[2] = vvp_net_ptr_t();
}

/*
 * the %force/link instruction connects a source node to a
 * destination node. The destination node must be a signal, as it is
 * marked with the source of the force so that it may later be
 * unlinked without specifically knowing the source that this
 * instruction used.
 */
bool of_FORCE_LINK(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*dst = cp->net;
      vvp_net_t*src = cp->net2;

      vvp_fun_signal_base*sig
	    = reinterpret_cast<vvp_fun_signal_base*>(dst->fun);
      assert(sig);

	/* Detect the special case that we are already forced the
	   source onto the destination. */
      if (sig->force_link == src)
	    return true;

	/* If there is a linked force already, then unlink it. */
      if (sig->force_link)
	    unlink_force(dst);

      sig->force_link = src;

	/* Link the output of the src to the port[2] (the force
	   port) of the destination. */
      vvp_net_ptr_t dst_ptr (dst, 2);
      dst->port[2] = src->out;
      src->out = dst_ptr;

      return true;
}

/*
 * The %force/v instruction invokes a force assign of a constant value
 * to a signal. The instruction arguments are:
 *
 *     %force/v <net>, <base>, <wid> ;
 *
 * where the <net> is the net label assembled into a vvp_net pointer,
 * and the <base> and <wid> are stashed in the bit_idx array.
 *
 * The instruction writes a vvp_vector4_t value to port-2 of the
 * target signal.
 */
bool of_FORCE_V(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net  = cp->net;
      unsigned  base = cp->bit_idx[0];
      unsigned  wid  = cp->bit_idx[1];

	/* Collect the thread bits into a vector4 item. */
      vvp_vector4_t value = vthread_bits_to_vector(thr, base, wid);

	/* set the value into port 1 of the destination. */
      vvp_net_ptr_t ptr (net, 2);
      vvp_send_vec4(ptr, value);

      return true;
}

bool of_FORCE_X0(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];

	// Implicitly, we get the base into the target vector from the
	// X0 register.
      long index = thr->words[0].w_int;

      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (net->fun);

      if (index < 0 && (wid <= (unsigned)-index))
	    return true;

      if (index >= (long)sig->size())
	    return true;

      if (index < 0) {
	    wid -= (unsigned) -index;
	    index = 0;
      }

      if (index+wid > sig->size())
	    wid = sig->size() - index;

      vvp_vector4_t bit_vec(wid);
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_bit4_t bit_val = thr_get_bit(thr, bit);
	    bit_vec.set_bit(idx, bit_val);
	    if (bit >= 4)
		  bit += 1;
      }

      vvp_net_ptr_t ptr (net, 2);
      vvp_send_vec4_pv(ptr, bit_vec, index, wid, sig->size());

      return true;
}
/*
 * The %fork instruction causes a new child to be created and pushed
 * in front of any existing child. This causes the new child to be the
 * parent of any previous children, and for me to be the parent of the
 * new child.
 */
bool of_FORK(vthread_t thr, vvp_code_t cp)
{
      vthread_t child = vthread_new(cp->cptr2, cp->scope);

      child->child  = thr->child;
      child->parent = thr;
      thr->child = child;
      if (child->child) {
	    assert(child->child->parent == thr);
	    child->child->parent = child;
      }

      thr->fork_count += 1;

      schedule_vthread(child, 0, true);
      return true;
}

bool of_INV(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      for (unsigned idx = 0 ;  idx < cp->bit_idx[1] ;  idx += 1) {
	    vvp_bit4_t val = thr_get_bit(thr, cp->bit_idx[0]+idx);
	    switch (val) {
		case BIT4_0:
		  val = BIT4_1;
		  break;
		case BIT4_1:
		  val = BIT4_0;
		  break;
		default:
		  val = BIT4_X;
		  break;
	    }
	    thr_put_bit(thr, cp->bit_idx[0]+idx, val);
      }
      return true;
}


/*
** Index registers, unsigned arithmetic.
*/

bool of_IX_ADD(vthread_t thr, vvp_code_t cp)
{
  thr->words[cp->bit_idx[0] & 3].w_int += cp->number;
  return true;
}

bool of_IX_SUB(vthread_t thr, vvp_code_t cp)
{
  thr->words[cp->bit_idx[0] & 3].w_int -= cp->number;
  return true;
}

bool of_IX_MUL(vthread_t thr, vvp_code_t cp)
{
  thr->words[cp->bit_idx[0] & 3].w_int *= cp->number;
  return true;
}

bool of_IX_LOAD(vthread_t thr, vvp_code_t cp)
{
      thr->words[cp->bit_idx[0] & 3].w_int = cp->number;
      return true;
}

/*
 * Load a vector into an index register. The format of the
 * opcode is:
 *
 *   %ix/get <ix>, <base>, <wid>
 *
 * where <ix> is the index register, <base> is the base of the
 * vector and <wid> is the width in bits.
 *
 * Index registers only hold binary values, so if any of the
 * bits of the vector are x or z, then set the value to 0,
 * set bit[4] to 1, and give up.
 */
bool of_IX_GET(vthread_t thr, vvp_code_t cp)
{
      unsigned index = cp->bit_idx[0];
      unsigned base  = cp->bit_idx[1];
      unsigned width = cp->number;

      unsigned long v = 0;
      bool unknown_flag = false;

      for (unsigned i = 0 ;  i<width ;  i += 1) {
	    vvp_bit4_t vv = thr_get_bit(thr, base);
	    if (bit4_is_xz(vv)) {
		  v = 0UL;
		  unknown_flag = true;
		  break;
	    }

	    v |= (unsigned long) vv << i;

	    if (base >= 4)
		  base += 1;
      }
      thr->words[index].w_int = v;

	/* Set bit 4 as a flag if the input is unknown. */
      thr_put_bit(thr, 4, unknown_flag? BIT4_1 : BIT4_0);

      return true;
}

bool of_IX_GET_S(vthread_t thr, vvp_code_t cp)
{
      unsigned index = cp->bit_idx[0];
      unsigned base  = cp->bit_idx[1];
      unsigned width = cp->number;

      unsigned long v = 0;
      bool unknown_flag = false;

      vvp_bit4_t vv = BIT4_0;
      for (unsigned i = 0 ;  i<width ;  i += 1) {
	    vv = thr_get_bit(thr, base);
	    if (bit4_is_xz(vv)) {
		  v = 0UL;
		  unknown_flag = true;
		  break;
	    }

	    v |= (unsigned long) vv << i;

	    if (base >= 4)
		  base += 1;
      }

	/* Sign-extend to fill the integer value. */
      if (!unknown_flag) {
	    unsigned long pad = vv;
	    for (unsigned i = width ; i < 8*sizeof(v) ;  i += 1) {
		  v |= pad << i;
	    }
      }

      thr->words[index].w_int = v;

	/* Set bit 4 as a flag if the input is unknown. */
      thr_put_bit(thr, 4, unknown_flag? BIT4_1 : BIT4_0);

      return true;
}


/*
 * The various JMP instruction work simply by pulling the new program
 * counter from the instruction and resuming. If the jump is
 * conditional, then test the bit for the expected value first.
 */
bool of_JMP(vthread_t thr, vvp_code_t cp)
{
      thr->pc = cp->cptr;

	/* Normally, this returns true so that the processor just
	   keeps going to the next instruction. However, if there was
	   a $stop or vpiStop, returning false here can break the
	   simulation out of a hung loop. */
      if (schedule_stopped()) {
	    schedule_vthread(thr, 0, false);
	    return false;
      }

      return true;
}

bool of_JMP0(vthread_t thr, vvp_code_t cp)
{
      if (thr_get_bit(thr, cp->bit_idx[0]) == 0)
	    thr->pc = cp->cptr;

	/* Normally, this returns true so that the processor just
	   keeps going to the next instruction. However, if there was
	   a $stop or vpiStop, returning false here can break the
	   simulation out of a hung loop. */
      if (schedule_stopped()) {
	    schedule_vthread(thr, 0, false);
	    return false;
      }

      return true;
}

bool of_JMP0XZ(vthread_t thr, vvp_code_t cp)
{
      if (thr_get_bit(thr, cp->bit_idx[0]) != BIT4_1)
	    thr->pc = cp->cptr;

	/* Normally, this returns true so that the processor just
	   keeps going to the next instruction. However, if there was
	   a $stop or vpiStop, returning false here can break the
	   simulation out of a hung loop. */
      if (schedule_stopped()) {
	    schedule_vthread(thr, 0, false);
	    return false;
      }

      return true;
}

bool of_JMP1(vthread_t thr, vvp_code_t cp)
{
      if (thr_get_bit(thr, cp->bit_idx[0]) == 1)
	    thr->pc = cp->cptr;

	/* Normally, this returns true so that the processor just
	   keeps going to the next instruction. However, if there was
	   a $stop or vpiStop, returning false here can break the
	   simulation out of a hung loop. */
      if (schedule_stopped()) {
	    schedule_vthread(thr, 0, false);
	    return false;
      }

      return true;
}

/*
 * The %join instruction causes the thread to wait for the one and
 * only child to die.  If it is already dead (and a zombie) then I
 * reap it and go on. Otherwise, I tell the child that I am ready for
 * it to die, and it will reschedule me when it does.
 */
bool of_JOIN(vthread_t thr, vvp_code_t cp)
{
      assert(thr->child);
      assert(thr->child->parent == thr);

      assert(thr->fork_count > 0);


	/* If the child has already ended, reap it now. */
      if (thr->child->i_have_ended) {
	    thr->fork_count -= 1;
	    vthread_reap(thr->child);
	    return true;
      }

	/* Otherwise, I get to start waiting. */
      thr->child->schedule_parent_on_end = 1;
      return false;
}

/*
 * %load/av <bit>, <array-label>, <wid> ;
 *
 * <bit> is the thread bit address for the result
 * <array-label> is the array to access, and
 * <wid> is the width of the word to read.
 *
 * The address of the word in the array is in index register 3.
 */
bool of_LOAD_AV(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];
      unsigned adr = thr->words[3].w_int;

      vvp_vector4_t word = array_get_word(cp->array, adr);

      if (word.size() != wid) {
	    fprintf(stderr, "internal error: array width=%u, word.size()=%u, wid=%u\n",
		    0, word.size(), wid);
      }
      assert(word.size() == wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1, bit += 1) {
	    vvp_bit4_t val = word.value(idx);
	    thr_put_bit(thr, bit, val);
      }

      return true;
}

/*
 * %load/avx.p <bit>, <array-label>, <idx> ;
 *
 * <bit> is the thread bit address for the result
 * <array-label> is the array to access, and
 * <wid> is the width of the word to read.
 *
 * The address of the word in the array is in index register 3.
 */
bool of_LOAD_AVX_P(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned index = cp->bit_idx[1];
      unsigned adr = thr->words[3].w_int;

      unsigned use_index = thr->words[index].w_int;

      vvp_vector4_t word = array_get_word(cp->array, adr);

      if (use_index >= word.size()) {
	    thr_put_bit(thr, bit, BIT4_X);
      } else {
	    thr_put_bit(thr, bit, word.value(use_index));
      }

      thr->words[index].w_int = use_index + 1;

      return true;
}

/*
 * %load/mv <bit>, <mem-label>, <wid> ;
 *
 * <bit> is the thread bit address for the result
 * <mem-label> is the memory device to access, and
 * <wid> is the width of the word to read.
 *
 * The address of the word in the memory is in index register 3.
 */
bool of_LOAD_MV(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];
      unsigned adr = thr->words[3].w_int;

      vvp_vector4_t word = memory_get_word(cp->mem, adr);

      if (word.size() != wid) {
	    fprintf(stderr, "internal error: mem width=%u, word.size()=%u, wid=%u\n",
		    memory_word_width(cp->mem), word.size(), wid);
      }
      assert(word.size() == wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1, bit += 1) {
	    vvp_bit4_t val = word.value(idx);
	    thr_put_bit(thr, bit, val);
      }

      return true;
}


/*
 * %load/nx <bit>, <vpi-label>, <idx>  ; Load net/indexed.
 *
 * cp->bit_idx[0] contains the <bit> value, an index into the thread
 * bit register.
 *
 * cp->bin_idx[1] is the <idx> value from the words array.
 *
 * cp->handle is the linked reference to the __vpiSignal that we are
 * to read from.
 */
bool of_LOAD_NX(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      assert(cp->bit_idx[1] <  4);
      assert(cp->handle->vpi_type->type_code == vpiNet);

      struct __vpiSignal*sig =
	    reinterpret_cast<struct __vpiSignal*>(cp->handle);

      unsigned idx = thr->words[cp->bit_idx[1]].w_int;

      vvp_fun_signal_vec*fun = dynamic_cast<vvp_fun_signal_vec*>(sig->node->fun);
      assert(sig != 0);

      vvp_bit4_t val = fun->value(idx);
      thr_put_bit(thr, cp->bit_idx[0], val);

      return true;
}

/* %load/v <bit>, <label>, <wid>
 *
 * Implement the %load/v instruction. Load the vector value of the
 * requested width from the <label> functor starting in the thread bit
 * <bit>.
 *
 * The <bit> value is the destination in the thread vector store, and
 * is in cp->bit_idx[0].
 *
 * The <wid> value is the expected with of the vector, and is in
 * cp->bit_idx[1].
 *
 * The functor to read from is the vvp_net_t object pointed to by the
 * cp->net pointer.
 */
bool of_LOAD_VEC(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      assert(cp->bit_idx[1] > 0);

      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];
      vvp_net_t*net = cp->net;

	/* For the %load to work, the functor must actually be a
	   signal functor. Only signals save their vector value. */
      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (net->fun);
      if (sig == 0) {
	    cerr << "%%load/v error: Net arg not a vector signal? "
		 << typeid(*net->fun).name() << endl;
      }
      assert(sig);

      vvp_vector4_t sig_value = sig->vec4_value();
      sig_value.resize(wid);

	/* Check the address once, before we scan the vector. */
      thr_check_addr(thr, bit+wid-1);

	/* Copy the vector bits into the bits4 vector. Do the copy
	   directly to skip the excess calls to thr_check_addr. */
      thr->bits4.set_vec(bit, sig_value);

      return true;
}

bool of_LOAD_WR(vthread_t thr, vvp_code_t cp)
{
      struct __vpiHandle*tmp = cp->handle;
      t_vpi_value val;

      val.format = vpiRealVal;
      vpi_get_value(tmp, &val);

      thr->words[cp->bit_idx[0]].w_real = val.value.real;

      return true;
}

/*
 * %load/x <bit>, <functor>, <index>
 *
 * <bit> is the destination thread bit and must be >= 4.
 */
bool of_LOAD_X(vthread_t thr, vvp_code_t cp)
{
	// <bit> is the thread bit to load
      assert(cp->bit_idx[0] >= 4);
      unsigned bit = cp->bit_idx[0];

	// <index> is the index register to use. The actual index into
	// the vector is the value of the index register.
      unsigned index_idx = cp->bit_idx[1];
      unsigned index = thr->words[index_idx].w_int;

	// <functor> is converted to a vvp_net_t pointer from which we
	// read our value.
      vvp_net_t*net = cp->net;

	// For the %load to work, the functor must actually be a
	// signal functor. Only signals save their vector value.
      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (net->fun);
      assert(sig);

      vvp_bit4_t val = index >= sig->size()? BIT4_X : sig->value(index);
      thr_put_bit(thr, bit, val);

      return true;
}

bool of_LOAD_XP(vthread_t thr, vvp_code_t cp)
{
	// First do the normal handling of the %load/x
      of_LOAD_X(thr, cp);

	// Now do the post-increment
      unsigned index_idx = cp->bit_idx[1];
      thr->words[index_idx].w_int += 1;

      return true;
}

bool of_LOADI_WR(vthread_t thr, vvp_code_t cp)
{
      unsigned idx = cp->bit_idx[0];
      double mant = cp->number;
      int exp = cp->bit_idx[1];

	// Detect +infinity
      if (exp==0x3fff && cp->number==0) {
	    thr->words[idx].w_real = INFINITY;
	    return true;
      }
	// Detect -infinity
      if (exp==0x7fff && cp->number==0) {
	    thr->words[idx].w_real = -INFINITY;
	    return true;
      }
	// Detect NaN
      if ( (exp&0x3fff) == 0x3fff ) {
	    thr->words[idx].w_real = nan("");
      }

      double sign = (exp & 0x4000)? -1.0 : 1.0;

      exp &= 0x1fff;

      mant = sign * ldexp(mant, exp - 0x1000);
      thr->words[idx].w_real = mant;
      return true;
}

static void do_verylong_mod(vthread_t thr, vvp_code_t cp,
			    bool left_is_neg, bool right_is_neg)
{
      bool out_is_neg = left_is_neg != right_is_neg;
      int len=cp->number;
      unsigned char *a, *z, *t;
      a = new unsigned char[len+1];
      z = new unsigned char[len+1];
      t = new unsigned char[len+1];

      unsigned char carry;
      unsigned char temp;

      int mxa = -1, mxz = -1;
      int i;
      int current, copylen;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      unsigned lb_carry = left_is_neg? 1 : 0;
      unsigned rb_carry = right_is_neg? 1 : 0;
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb | rb) & 2) {
		  delete []t;
		  delete []z;
		  delete []a;
		  goto x_out;
	    }

	    if (left_is_neg) {
		  lb = (1-lb) + lb_carry;
		  lb_carry = (lb & ~1)? 1 : 0;
		  lb &= 1;
	    }
	    if (right_is_neg) {
		  rb = (1-rb) + rb_carry;
		  rb_carry = (rb & ~1)? 1 : 0;
		  rb &= 1;
	    }

	    z[idx]=lb;
	    a[idx]=1-rb;	// for 2s complement add..

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      z[len]=0;
      a[len]=1;

      for(i=len-1;i>=0;i--) {
	    if(!a[i]) {
		  mxa=i;
		  break;
	    }
      }

      for(i=len-1;i>=0;i--) {
	    if(z[i]) {
		  mxz=i;
		  break;
	    }
      }

      if((mxa>mxz)||(mxa==-1)) {
	    if(mxa==-1) {
		  delete []t;
		  delete []z;
		  delete []a;
		  goto x_out;
	    }

	    goto tally;
      }

      copylen = mxa + 2;
      current = mxz - mxa;

      while(current > -1) {
	    carry = 1;
	    for(i=0;i<copylen;i++) {
		  temp = z[i+current] + a[i] + carry;
		  t[i] = (temp&1);
		  carry = (temp>>1);
	    }

	    if(carry) {
		  for(i=0;i<copylen;i++) {
			z[i+current] = t[i];
		  }
	    }

	    current--;
      }

 tally:

      carry = out_is_neg? 1 : 0;
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned ob = z[idx];
	    if (out_is_neg) {
		  ob = (1-ob) + carry;
		  carry = (ob & ~1)? 1 : 0;
		  ob = ob & 1;
	    }
	    thr_put_bit(thr, cp->bit_idx[0]+idx, ob?BIT4_1:BIT4_0);
      }

      delete []t;
      delete []z;
      delete []a;
      return;

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return;
}

bool of_MOD(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if(cp->number <= 8*sizeof(unsigned long long)) {
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned idx2 = cp->bit_idx[1];
	    unsigned long long lv = 0, rv = 0;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  unsigned long long lb = thr_get_bit(thr, idx1);
		  unsigned long long rb = thr_get_bit(thr, idx2);

		  if ((lb | rb) & 2)
			goto x_out;

		  lv |= (unsigned long long) lb << idx;
		  rv |= (unsigned long long) rb << idx;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	    if (rv == 0)
		  goto x_out;

	    lv %= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1)?BIT4_1 : BIT4_0);
		  lv >>= 1;
	    }

	    return true;

      } else {
	    do_verylong_mod(thr, cp, false, false);
	    return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

bool of_MOD_S(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

	/* Handle the case that we can fit the bits into a long-long
	   variable. We cause use native % to do the work. */
      if(cp->number <= 8*sizeof(long long)) {
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned idx2 = cp->bit_idx[1];
	    long long lv = 0, rv = 0;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  long long lb = thr_get_bit(thr, idx1);
		  long long rb = thr_get_bit(thr, idx2);

		  if ((lb | rb) & 2)
			goto x_out;

		  lv |= (long long) lb << idx;
		  rv |= (long long) rb << idx;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	    if (rv == 0)
		  goto x_out;

	      /* Sign extend the signed operands. */
	    if (lv & (1LL << (cp->number-1)))
		  lv |= -1LL << cp->number;
	    if (rv & (1LL << (cp->number-1)))
		  rv |= -1LL << cp->number;

	    lv %= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1)?BIT4_1:BIT4_0);
		  lv >>= 1;
	    }

	    return true;

      } else {

	    bool left_is_neg
		  = thr_get_bit(thr,cp->bit_idx[0]+cp->number-1) == 1;
	    bool right_is_neg
		  = thr_get_bit(thr,cp->bit_idx[1]+cp->number-1) == 1;
	    do_verylong_mod(thr, cp, left_is_neg, right_is_neg);
	    return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

/*
 * %mod/wr <dest>, <src>
 */
bool of_MOD_WR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = fmod(l,r);

      return true;
}

/*
 * %mov <dest>, <src>, <wid>
 *   This instruction is implemented by the of_MOV function
 *   below. However, during runtime vvp might notice that the
 *   parameters have certain properties that make it possible to
 *   replace the of_MOV opcode with a more specific instruction that
 *   more directly does the job. All the of_MOV*_ functions are
 *   functions that of_MOV might use to replace itself.
 */

static bool of_MOV1XZ_(vthread_t thr, vvp_code_t cp)
{
      thr_check_addr(thr, cp->bit_idx[0]+cp->number-1);
      vvp_vector4_t tmp (cp->number, (vvp_bit4_t)cp->bit_idx[1]);
      thr->bits4.set_vec(cp->bit_idx[0], tmp);
      return true;
}

static bool of_MOV_(vthread_t thr, vvp_code_t cp)
{
	/* This variant implements the general case that we know
	   neither the source nor the destination to be <4. Otherwise,
	   we copy all the bits manually. */

      thr_check_addr(thr, cp->bit_idx[0]+cp->number);
      thr_check_addr(thr, cp->bit_idx[1]+cp->number);
	// Read the source vector out
      vvp_vector4_t tmp (thr->bits4, cp->bit_idx[1], cp->number);
	// Write it in the new place.
      thr->bits4.set_vec(cp->bit_idx[0], tmp);

      return true;
}

bool of_MOV(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->bit_idx[1] >= 4) {
	    cp->opcode = &of_MOV_;
	    return cp->opcode(thr, cp);

      } else {
	    cp->opcode = &of_MOV1XZ_;
	    return cp->opcode(thr, cp);
      }

      return true;
}

/*
*  %mov/wr <dst>, <src>
*/
bool of_MOV_WR(vthread_t thr, vvp_code_t cp)
{
      unsigned dst = cp->bit_idx[0];
      unsigned src = cp->bit_idx[1];

      thr->words[dst].w_real = thr->words[src].w_real;
      return true;
}

bool of_MOVI(vthread_t thr, vvp_code_t cp)
{
      unsigned dst = cp->bit_idx[0];
      unsigned val = cp->bit_idx[1];
      unsigned wid = cp->number;

      thr_check_addr(thr, dst+wid);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1, val >>= 1)
	    thr->bits4.set_bit(dst+idx, (val&1)? BIT4_1 : BIT4_0);

      return true;
}

bool of_MUL(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      if(cp->number <= 8*sizeof(unsigned long)) {

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned long lv = 0, rv = 0;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if (bit4_is_xz(lb) || bit4_is_xz(rb))
		  goto x_out;

	    lv |= (unsigned long) lb << idx;
	    rv |= (unsigned long) rb << idx;

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      lv *= rv;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? BIT4_1 : BIT4_0);
	    lv >>= 1;
      }

      return true;
      } else {
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      unsigned char *a, *b, *sum;
      a = new unsigned char[cp->number];
      b = new unsigned char[cp->number];
      sum = new unsigned char[cp->number];

      int mxa = -1;
      int mxb = -1;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if (bit4_is_xz(lb) || bit4_is_xz(rb))
		  {
                  delete[]sum;
                  delete[]b;
                  delete[]a;
		  goto x_out;
		  }

	    if((a[idx] = lb)) mxa=idx+1;
	    if((b[idx] = rb)) mxb=idx;
            sum[idx]=0;

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

//    do "unsigned ZZ sum = a * b" the hard way..
      for(int i=0;i<=mxb;i++)
                {
                if(b[i])
                        {
                        unsigned char carry=0;
                        unsigned char temp;

                        for(int j=0;j<=mxa;j++)
                                {
                                if(i+j>=(int)cp->number) break;
                                temp=sum[i+j]+a[j]+carry;
                                sum[i+j]=(temp&1);
                                carry=(temp>>1);
                                }
                        }
                }

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    thr_put_bit(thr, cp->bit_idx[0]+idx, sum[idx]?BIT4_1:BIT4_0);
      }

      delete[]sum;
      delete[]b;
      delete[]a;
      return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

bool of_MUL_WR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = l * r;

      return true;
}

bool of_MULI(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

	/* If the value fits into a native unsigned long, then make an
	   unsigned long variable with the numbers, to a native
	   multiply, and work with that. */

      if(cp->number <= 8*sizeof(unsigned long)) {
	    unsigned idx1 = cp->bit_idx[0];
	    unsigned long lv = 0, rv = cp->bit_idx[1];

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  vvp_bit4_t lb = thr_get_bit(thr, idx1);

		  if (bit4_is_xz(lb))
			goto x_out;

		  lv |= (unsigned long) lb << idx;

		  idx1 += 1;
	    }

	    lv *= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1)? BIT4_1 : BIT4_0);
		  lv >>= 1;
	    }

	    return true;
      }

	/* number is too large for local long, so do bitwise
	   multiply. */

      unsigned idx1; idx1 = cp->bit_idx[0];
      unsigned imm;  imm  = cp->bit_idx[1];

      unsigned char *a, *b, *sum;
      a = new unsigned char[cp->number];
      b = new unsigned char[cp->number];
      sum = new unsigned char[cp->number];

      int mxa; mxa = -1;
      int mxb; mxb = -1;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = (imm & 1)? BIT4_1 : BIT4_0;

	    imm >>= 1;

	    if (bit4_is_xz(lb)) {
                  delete[]sum;
                  delete[]b;
                  delete[]a;
		  goto x_out;
	    }

	    if((a[idx] = lb)) mxa=idx+1;
	    if((b[idx] = rb)) mxb=idx;
            sum[idx]=0;

	    idx1 += 1;
      }

//    do "unsigned ZZ sum = a * b" the hard way..
      for(int i=0;i<=mxb;i++) {
	    if(b[i]) {
		  unsigned char carry=0;
		  unsigned char temp;

		  for(int j=0;j<=mxa;j++) {
			if(i+j>=(int)cp->number) break;
			temp=sum[i+j]+a[j]+carry;
			sum[i+j]=(temp&1);
			carry=(temp>>1);
		  }
	    }
      }


      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    thr_put_bit(thr, cp->bit_idx[0]+idx, sum[idx]?BIT4_1:BIT4_0);
      }

      delete[]sum;
      delete[]b;
      delete[]a;

      return true;

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

bool of_NAND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if ((lb == BIT4_0) || (rb == BIT4_0)) {
		  thr_put_bit(thr, idx1, BIT4_1);

	    } else if ((lb == BIT4_1) && (rb == BIT4_1)) {
		  thr_put_bit(thr, idx1, BIT4_0);

	    } else {
		  thr_put_bit(thr, idx1, BIT4_X);
	    }

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}


bool of_NOOP(vthread_t thr, vvp_code_t cp)
{
      return true;
}

bool of_NORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      vvp_bit4_t lb = BIT4_1;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t rb = thr_get_bit(thr, idx2+idx);
	    if (rb == BIT4_1) {
		  lb = BIT4_0;
		  break;
	    }

	    if (rb != BIT4_0)
		  lb = BIT4_X;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_ANDR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      vvp_bit4_t lb = BIT4_1;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t rb = thr_get_bit(thr, idx2+idx);
	    if (rb == BIT4_0) {
		  lb = BIT4_0;
		  break;
	    }

	    if (rb != BIT4_1)
		  lb = BIT4_X;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_NANDR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      vvp_bit4_t lb = BIT4_0;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t rb = thr_get_bit(thr, idx2+idx);
	    if (rb == BIT4_0) {
		  lb = BIT4_1;
		  break;
	    }

	    if (rb != BIT4_1)
		  lb = BIT4_X;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_ORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      vvp_bit4_t lb = BIT4_0;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t rb = thr_get_bit(thr, idx2+idx);
	    if (rb == BIT4_1) {
		  lb = BIT4_1;
		  break;
	    }

	    if (rb != BIT4_0)
		  lb = BIT4_X;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_XORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      vvp_bit4_t lb = BIT4_0;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t rb = thr_get_bit(thr, idx2+idx);
	    if (rb == BIT4_1)
		  lb = ~lb;
	    else if (rb != BIT4_0) {
		  lb = BIT4_X;
		  break;
	    }
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_XNORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      vvp_bit4_t lb = BIT4_1;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t rb = thr_get_bit(thr, idx2+idx);
	    if (rb == BIT4_1)
		  lb = ~lb;
	    else if (rb != BIT4_0) {
		  lb = BIT4_X;
		  break;
	    }
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_OR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if ((lb == BIT4_1) || (rb == BIT4_1)) {
		  thr_put_bit(thr, idx1, BIT4_1);

	    } else if ((lb == BIT4_0) && (rb == BIT4_0)) {
		  thr_put_bit(thr, idx1, BIT4_0);

	    } else {
		  thr_put_bit(thr, idx1, BIT4_X);
	    }

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

bool of_NOR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if ((lb == BIT4_1) || (rb == BIT4_1)) {
		  thr_put_bit(thr, idx1, BIT4_0);

	    } else if ((lb == BIT4_0) && (rb == BIT4_0)) {
		  thr_put_bit(thr, idx1, BIT4_1);

	    } else {
		  thr_put_bit(thr, idx1, BIT4_X);
	    }

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

/*
 * These implement the %release/net and %release/reg instructions. The
 * %release/net instruction applies to a net kind of functor by
 * sending the release/net command to the command port. (See vvp_net.h
 * for details.) The %release/reg instruction is the same, but sends
 * the release/reg command instead. These are very similar to the
 * %deassign instruction.
 */
bool of_RELEASE_NET(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;

      vvp_fun_signal_base*sig = reinterpret_cast<vvp_fun_signal_base*>(net->fun);
      assert(sig);

	/* XXXX Release for %force/link not yet implemented. */
      if (sig->force_link)
	    unlink_force(net);
      assert(sig->force_link == 0);

      vvp_net_ptr_t ptr (net, 3);
      vvp_send_long(ptr, 2);

      return true;
}


bool of_RELEASE_REG(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;

      vvp_fun_signal_base*sig = reinterpret_cast<vvp_fun_signal_base*>(net->fun);
      assert(sig);

	// This is the net that is forcing me...
      if (vvp_net_t*src = sig->force_link) {
	      // And this is the pointer to be removed.
	    vvp_net_ptr_t dst_ptr (net, 2);
	    unlink_from_driver(src, dst_ptr);
     }

	// Send a command to this signal to unforce itself.
      vvp_net_ptr_t ptr (net, 3);
      vvp_send_long(ptr, 3);

      return true;
}


/*
 * This implements the "%set/av <label>, <bit>, <wid>" instruction. In
 * this case, the <label> is an array label, and the <bit> and <wid>
 * are the thread vector of a value to be written in.
 */
bool of_SET_AV(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];
      unsigned off = thr->words[1].w_int;
      unsigned adr = thr->words[3].w_int;

	/* Make a vector of the desired width. */
      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      array_set_word(cp->array, adr, off, value);
      return true;
}

/*
 * This implements the "%set/mv <label>, <bit>, <wid>" instruction. In
 * this case, the <label> is a memory label, and the <bit> and <wid>
 * are the thread vector of a value to be written in.
 */
bool of_SET_MV(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];
      unsigned off = thr->words[1].w_int;
      unsigned adr = thr->words[3].w_int;

	/* Make a vector of the desired width. */
      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      memory_set_word(cp->mem, adr, off, value);
      return true;
}


/*
 * This implements the "%set/v <label>, <bit>, <wid>" instruction.
 *
 * The <label> is a reference to a vvp_net_t object, and it is in
 * cp->net.
 *
 * The <bit> is the thread bit address, and is in cp->bin_idx[0].
 *
 * The <wid> is the width of the vector I'm to make, and is in
 * cp->bin_idx[1].
 */
bool of_SET_VEC(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[1] > 0);
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];

	/* set the value into port 0 of the destination. */
      vvp_net_ptr_t ptr (cp->net, 0);

      if (bit >= 4) {
	    vvp_vector4_t value(thr->bits4,bit,wid);
	    vvp_send_vec4(ptr, value);

      } else {
	      /* Make a vector of the desired width. */
	    vvp_bit4_t bit_val = (vvp_bit4_t)bit;
	    vvp_vector4_t value(wid, bit_val);

	    vvp_send_vec4(ptr, value);
      }

      return true;
}

bool of_SET_WORDR(vthread_t thr, vvp_code_t cp)
{
      struct __vpiHandle*tmp = cp->handle;
      t_vpi_value val;

      val.format = vpiRealVal;
      val.value.real = thr->words[cp->bit_idx[0]].w_real;
      vpi_put_value(tmp, &val, 0, vpiNoDelay);

      return true;
}

/*
 * Implement the %set/x instruction:
 *
 *      %set/x <functor>, <bit>, <wid>
 *
 * The bit value of a vector go into the addressed functor. Do not
 * transfer bits that are outside the signal range. Get the target
 * vector dimensions from the vvp_fun_signal addressed by the vvp_net
 * pointer.
 */
bool of_SET_X0(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];

	// Implicitly, we get the base into the target vector from the
	// X0 register.
      long index = thr->words[0].w_int;

      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (net->fun);

      if (index < 0 && (wid <= (unsigned)-index))
	    return true;

      if (index >= (long)sig->size())
	    return true;

      if (index < 0) {
	    wid -= (unsigned) -index;
	    index = 0;
      }

      if (index+wid > sig->size())
	    wid = sig->size() - index;

      vvp_vector4_t bit_vec(wid);
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_bit4_t bit_val = thr_get_bit(thr, bit);
	    bit_vec.set_bit(idx, bit_val);
	    if (bit >= 4)
		  bit += 1;
      }

      vvp_net_ptr_t ptr (net, 0);
      vvp_send_vec4_pv(ptr, bit_vec, index, wid, sig->size());

      return true;
}

bool of_SHIFTL_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;

      assert(base >= 4);
      thr_check_addr(thr, base+wid-1);

      if (shift >= wid) {
	      // Shift is so far that all value is shifted out. Write
	      // in a constant 0 result.
	    vvp_vector4_t tmp (wid, BIT4_0);
	    thr->bits4.set_vec(base, tmp);

      } else if (shift > 0) {
	    vvp_vector4_t tmp (thr->bits4, base, wid-shift);
	    thr->bits4.set_vec(base+shift, tmp);

	      // Fill zeros on the bottom
	    vvp_vector4_t fil (shift, BIT4_0);
	    thr->bits4.set_vec(base, fil);
      }
      return true;
}

/*
 * This is an unsigned right shift:
 *
 *    %shiftr/i0 <bit>, <wid>
 *
 * The vector at address <bit> with width <wid> is shifted right a
 * number of bits stored in index/word register 0.
 */
bool of_SHIFTR_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;

      if (shift > 0) {
	    unsigned idx;
	    for (idx = 0 ;  (idx+shift) < wid ;  idx += 1) {
		  unsigned src = base + idx + shift;
		  unsigned dst = base + idx;
		  thr_put_bit(thr, dst, thr_get_bit(thr, src));
	    }
	    for ( ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, BIT4_0);
      }
      return true;
}

bool of_SHIFTR_S_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;
      vvp_bit4_t sign = thr_get_bit(thr, base+wid-1);

      if (shift >= wid) {
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, sign);

      } else if (shift > 0) {
	    for (unsigned idx = 0 ;  idx < (wid-shift) ;  idx += 1) {
		  unsigned src = base + idx + shift;
		  unsigned dst = base + idx;
		  thr_put_bit(thr, dst, thr_get_bit(thr, src));
	    }
	    for (unsigned idx = (wid-shift) ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, sign);
      }
      return true;
}

bool of_SUB(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned long*lva = vector_to_array(thr, cp->bit_idx[0], cp->number);
      unsigned long*lvb = vector_to_array(thr, cp->bit_idx[1], cp->number);
      if (lva == 0 || lvb == 0)
	    goto x_out;


      unsigned carry;
      carry = 1;
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned long tmp;
	    unsigned sum = carry;

	    tmp = lva[idx/CPU_WORD_BITS];
	    sum += 1 &  (tmp >> (idx%CPU_WORD_BITS));

	    tmp = lvb[idx/CPU_WORD_BITS];
	    sum += 1 & ~(tmp >> (idx%CPU_WORD_BITS));

	    carry = sum / 2;
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (sum&1) ? BIT4_1 : BIT4_0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;
      delete[]lvb;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

bool of_SUB_WR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = l - r;
      return true;
}

bool of_SUBI(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned word_count = (cp->number+CPU_WORD_BITS-1)/CPU_WORD_BITS;

      unsigned long*lva = vector_to_array(thr, cp->bit_idx[0], cp->number);
      unsigned long*lvb;
      if (lva == 0)
	    goto x_out;

      lvb = new unsigned long[word_count];


      lvb[0] = cp->bit_idx[1];
      lvb[0] = ~lvb[0];
      for (unsigned idx = 1 ;  idx < word_count ;  idx += 1)
	    lvb[idx] = ~0UL;

      unsigned long carry;
      carry = 1;
      for (unsigned idx = 0 ;  (idx*CPU_WORD_BITS) < cp->number ;  idx += 1) {

	    unsigned long tmp = lvb[idx] + carry;
	    unsigned long sum = lva[idx] + tmp;
	    carry = 0UL;
	    if (tmp < lvb[idx])
		  carry = 1;
	    if (sum < tmp)
		  carry = 1;
	    if (sum < lva[idx])
		  carry = 1;
	    lva[idx] = sum;
      }

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned bit = lva[idx/CPU_WORD_BITS] >> (idx % CPU_WORD_BITS);
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (bit&1) ? BIT4_1 : BIT4_0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, BIT4_X);

      return true;
}

bool of_VPI_CALL(vthread_t thr, vvp_code_t cp)
{
	// printf("thread %p: %%vpi_call\n", thr);
      vpip_execute_vpi_call(thr, cp->handle);

      if (schedule_stopped()) {
	    if (! schedule_finished())
		  schedule_vthread(thr, 0, false);

	    return false;
      }

      return schedule_finished()? false : true;
}

/* %wait <label>;
 * Implement the wait by locating the vvp_net_T for the event, and
 * adding this thread to the threads list for the event. The some
 * argument is the  reference to the functor to wait for. This must be
 * an event object of some sort.
 */
bool of_WAIT(vthread_t thr, vvp_code_t cp)
{
      assert(! thr->waiting_for_event);
      thr->waiting_for_event = 1;

      vvp_net_t*net = cp->net;
	/* Get the functor as a waitable_hooks_s object. */
      waitable_hooks_s*ep = dynamic_cast<waitable_hooks_s*> (net->fun);
      assert(ep);
	/* Add this thread to the list in the event. */
      thr->wait_next = ep->threads;
      ep->threads = thr;
	/* Return false to suspend this thread. */
      return false;
}


bool of_XNOR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);
	    thr_put_bit(thr, idx1, ~(lb ^ rb));

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}


bool of_XOR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);

	    if ((lb == BIT4_1) && (rb == BIT4_1)) {
		  thr_put_bit(thr, idx1, BIT4_0);

	    } else if ((lb == BIT4_0) && (rb == BIT4_0)) {
		  thr_put_bit(thr, idx1, BIT4_0);

	    } else if ((lb == BIT4_1) && (rb == BIT4_0)) {
		  thr_put_bit(thr, idx1, BIT4_1);

	    } else if ((lb == BIT4_0) && (rb == BIT4_1)) {
		  thr_put_bit(thr, idx1, BIT4_1);

	    } else {
		  thr_put_bit(thr, idx1, BIT4_X);
	    }

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}


bool of_ZOMBIE(vthread_t thr, vvp_code_t)
{
      thr->pc = codespace_null();
      if ((thr->parent == 0) && (thr->child == 0))
	    delete thr;

      return false;
}

/*
 * These are phantom opcode used to call user defined functions.
 * They are used in code generated by the .ufunc statement. They
 * contain a pointer to executable code of the function, and to a
 * ufunc_core object that has all the port information about the
 * function.
 */
bool of_FORK_UFUNC(vthread_t thr, vvp_code_t cp)
{
	/* Copy all the inputs to the ufunc object to the port
	   variables of the function. This copies all the values
	   atomically. */
      cp->ufunc_core_ptr->assign_bits_to_ports();

      assert(thr->child == 0);
      assert(thr->fork_count == 0);

	/* Create a temporary thread, and push its execution. This is
	   done so that the assign_bits_to_ports above is atomic with
	   this startup. */
      vthread_t child = vthread_new(cp->cptr, cp->ufunc_core_ptr->scope());

      child->child = 0;
      child->parent = thr;
      thr->child = child;

      thr->fork_count += 1;
      schedule_vthread(child, 0, true);

	/* After this function, the .ufunc code has placed an of_JOIN
	   to pause this thread. Since the child was pushed by the
	   flag to schecule_vthread, the called function starts up
	   immediately. */
      return true;
}

bool of_JOIN_UFUNC(vthread_t thr, vvp_code_t cp)
{
	/* Now copy the output from the result variable to the output
	   ports of the .ufunc device. */
      cp->ufunc_core_ptr->finish_thread(thr);

      return true;
}

/*
 * $Log: vthread.cc,v $
 */

