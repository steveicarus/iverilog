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
#ident "$Id: vthread.cc,v 1.122.2.1 2006/10/11 00:17:35 steve Exp $"
#endif

# include  "config.h"
# include  "vthread.h"
# include  "codes.h"
# include  "schedule.h"
# include  "functor.h"
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
      unsigned long *bits;

	/* These are the word registers. */
      union {
	    long w_int;
	    double w_real;
      } words[16];

      unsigned nbits;
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

#if SIZEOF_UNSIGNED_LONG == 8
# define THR_BITS_INIT 0xaaaaaaaaaaaaaaaaUL
#else
# define THR_BITS_INIT 0xaaaaaaaaUL
#endif

static void thr_check_addr(struct vthread_s*thr, unsigned addr)
{
      while (thr->nbits <= addr) {
	    unsigned word_cnt = thr->nbits/(CPU_WORD_BITS/2) + 1;
	    thr->bits = (unsigned long*)
		  realloc(thr->bits, word_cnt*sizeof(unsigned long));
	    thr->bits[word_cnt-1] = THR_BITS_INIT;
	    thr->nbits = word_cnt * (CPU_WORD_BITS/2);
      }
}

static inline unsigned thr_get_bit(struct vthread_s*thr, unsigned addr)
{
      assert(addr < thr->nbits);
      unsigned idx = addr % (CPU_WORD_BITS/2);
      addr /= (CPU_WORD_BITS/2);
      return (thr->bits[addr] >> (idx*2)) & 3UL;
}

static inline void thr_put_bit(struct vthread_s*thr,
			       unsigned addr, unsigned val)
{
      if (addr >= thr->nbits)
	    thr_check_addr(thr, addr);

      unsigned idx = addr % (CPU_WORD_BITS/2);
      addr /= (CPU_WORD_BITS/2);

      unsigned long mask = 3UL << (idx*2);
      unsigned long tmp = val;

      thr->bits[addr] = (thr->bits[addr] & ~mask) | (tmp << (idx*2));
}

static inline void thr_clr_bit_(struct vthread_s*thr, unsigned addr)
{
      unsigned idx = addr % (CPU_WORD_BITS/2);
      addr /= (CPU_WORD_BITS/2);

      unsigned long mask = 3UL << (idx*2);

      thr->bits[addr] &= ~mask;
}

unsigned vthread_get_bit(struct vthread_s*thr, unsigned addr)
{
      return thr_get_bit(thr, addr);
}

void vthread_put_bit(struct vthread_s*thr, unsigned addr, unsigned bit)
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
      unsigned long*val = new unsigned long[awid];

      for (unsigned idx = 0 ;  idx < awid ;  idx += 1)
	    val[idx] = 0;

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    unsigned long bit = thr_get_bit(thr, addr);

	    if (bit & 2)
		  goto x_out;

	    val[idx/CPU_WORD_BITS] |= bit << (idx % CPU_WORD_BITS);
	    if (addr >= 4)
		  addr += 1;
      }

      return val;

 x_out:
      delete[]val;
      return 0;
}


/*
 * Create a new thread with the given start address.
 */
vthread_t vthread_new(vvp_code_t pc, struct __vpiScope*scope)
{
      vthread_t thr = new struct vthread_s;
      thr->pc     = pc;
      thr->bits   = (unsigned long*)malloc(4 * sizeof(unsigned long));
      thr->bits[0] = THR_BITS_INIT;
      thr->bits[1] = THR_BITS_INIT;
      thr->bits[2] = THR_BITS_INIT;
      thr->bits[3] = THR_BITS_INIT;
      thr->nbits  = 4 * (CPU_WORD_BITS/2);
      thr->child  = 0;
      thr->parent = 0;
      thr->wait_next = 0;

	/* If the target scope never held a thread, then create a
	   header cell for it. This is a stub to make circular lists
	   easier to work with. */
      if (scope->threads == 0) {
	    scope->threads = new struct vthread_s;
	    scope->threads->pc = codespace_null();
	    scope->threads->bits   = 0;
	    scope->threads->nbits  = 0;
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

      thr_put_bit(thr, 0, 0);
      thr_put_bit(thr, 1, 1);
      thr_put_bit(thr, 2, 2);
      thr_put_bit(thr, 3, 3);

      return thr;
}

/*
 * Reaping pulls the thread out of the stack of threads. If I have a
 * child, then hand it over to my parent.
 */
static void vthread_reap(vthread_t thr)
{
      free(thr->bits);
      thr->bits = 0;

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

	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb == 0) || (rb == 0)) {
		  thr_put_bit(thr, idx1, 0);

	    } else if ((lb == 1) && (rb == 1)) {
		  thr_put_bit(thr, idx1, 1);

	    } else {
		  thr_put_bit(thr, idx1, 2);
	    }

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

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned bit = lva[idx/CPU_WORD_BITS] >> (idx % CPU_WORD_BITS);
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (bit&1) ? 1 : 0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;
      delete[]lvb;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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
      assert(cp->bit_idx[0] >= 4);

      unsigned word_count = (cp->number+CPU_WORD_BITS-1)/CPU_WORD_BITS;

      unsigned long*lva = vector_to_array(thr, cp->bit_idx[0], cp->number);
      unsigned long*lvb;
      if (lva == 0)
	    goto x_out;

      lvb = new unsigned long[word_count];

      lvb[0] = cp->bit_idx[1];
      for (unsigned idx = 1 ;  idx < word_count ;  idx += 1)
	    lvb[idx] = 0;

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

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned bit = lva[idx/CPU_WORD_BITS] >> (idx % CPU_WORD_BITS);
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (bit&1) ? 1 : 0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

      return true;
}

bool of_ASSIGN(vthread_t thr, vvp_code_t cp)
{
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[1]);
      schedule_assign(cp->iptr, bit_val, cp->bit_idx[0]);
      return true;
}

bool of_ASSIGN_D(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] < 4);
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[1]);
      schedule_assign(cp->iptr, bit_val, thr->words[cp->bit_idx[0]].w_int);
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

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_ipoint_t iptr = ipoint_index(cp->iptr, idx);

	    unsigned char bit_val = thr_get_bit(thr, bit);
	    schedule_assign(iptr, bit_val, delay);

	    if (bit >= 4)
		  bit += 1;
      }

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
      s_vpi_time del;

      del.type = vpiSimTime;
      vpip_time_to_timestruct(&del, schedule_simtime() + delay);

      struct __vpiHandle*tmp = cp->handle;

      t_vpi_value val;
      val.format = vpiRealVal;
      val.value.real = thr->words[cp->bit_idx[1]].w_real;
      vpi_put_value(tmp, &val, &del, vpiInertialDelay);

      return true;
}

bool of_ASSIGN_X0(vthread_t thr, vvp_code_t cp)
{
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[1]);
      vvp_ipoint_t itmp = ipoint_index(cp->iptr, thr->words[0].w_int);
      schedule_assign(itmp, bit_val, cp->bit_idx[0]);
      return true;
}

bool of_ASSIGN_MEM(vthread_t thr, vvp_code_t cp)
{
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[1]);
      schedule_memory(cp->mem, thr->words[3].w_int, bit_val, cp->bit_idx[0]);
      return true;
}

bool of_BLEND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if (lb != rb)
		  thr_put_bit(thr, idx1, 2);

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

bool of_CMPS(vthread_t thr, vvp_code_t cp)
{
      unsigned eq = 1;
      unsigned eeq = 1;
      unsigned lt = 0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      unsigned end1 = (idx1 < 4)? idx1 : idx1 + cp->number - 1;
      unsigned end2 = (idx2 < 4)? idx2 : idx2 + cp->number - 1;

      unsigned sig1 = thr_get_bit(thr, end1);
      unsigned sig2 = thr_get_bit(thr, end2);

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lv = thr_get_bit(thr, idx1);
	    unsigned rv = thr_get_bit(thr, idx2);

	    if (lv > rv) {
		  lt = 0;
		  eeq = 0;
	    } else if (lv < rv) {
		  lt = 1;
		  eeq = 0;
	    }
	    if (eq != 2) {
		  if ((lv == 0) && (rv != 0))
			eq = 0;
		  if ((lv == 1) && (rv != 1))
			eq = 0;
		  if ((lv | rv) >= 2)
			eq = 2;
	    }

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;
      }

      if (eq == 2)
	    lt = 2;
      else if ((sig1 == 1) && (sig2 == 0))
	    lt = 1;
      else if ((sig1 == 0) && (sig2 == 1))
	    lt = 0;

	/* Correct the lt bit to account for the sign of the parameters. */
      if (lt < 2) {
	    sig1 = thr_get_bit(thr, end1);
	    sig2 = thr_get_bit(thr, end2);

	      /* If both numbers are negative (and not equal) then
		 switch the direction of the lt. */
	    if ((sig1 == 1) && (sig2 == 1) && (eq != 1))
		  lt ^= 1;

	      /* If the first is negative and the last positive, then
		 a < b for certain. */
	    if ((sig1 == 1) && (sig2 == 0))
		  lt = 1;

	      /* If the first is positive and the last negative, then
		 a > b for certain. */
	    if ((sig1 == 0) && (sig2 == 1))
		  lt = 0;
      }

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPIU(vthread_t thr, vvp_code_t cp)
{
      unsigned eq = 1;
      unsigned eeq = 1;
      unsigned lt = 0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned imm  = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lv = thr_get_bit(thr, idx1);
	    unsigned rv = imm & 1;
	    imm >>= 1;

	    if (lv > rv) {
		  lt = 0;
		  eeq = 0;
	    } else if (lv < rv) {
		  lt = 1;
		  eeq = 0;
	    }
	    if (eq != 2) {
		  if ((lv == 0) && (rv != 0))
			eq = 0;
		  if ((lv == 1) && (rv != 1))
			eq = 0;
		  if ((lv | rv) >= 2)
			eq = 2;
	    }

	    if (idx1 >= 4) idx1 += 1;
      }

      if (eq == 2)
	    lt = 2;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPU(vthread_t thr, vvp_code_t cp)
{
      unsigned eq = 1;
      unsigned eeq = 1;
      unsigned lt = 0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lv = thr_get_bit(thr, idx1);
	    unsigned rv = thr_get_bit(thr, idx2);

	    if (lv > rv) {
		  lt = 0;
		  eeq = 0;
	    } else if (lv < rv) {
		  lt = 1;
		  eeq = 0;
	    }
	    if (eq != 2) {
		  if ((lv == 0) && (rv != 0))
			eq = 0;
		  if ((lv == 1) && (rv != 1))
			eq = 0;
		  if ((lv | rv) >= 2)
			eq = 2;
	    }

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;
      }

      if (eq == 2)
	    lt = 2;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPX(vthread_t thr, vvp_code_t cp)
{
      unsigned eq = 1;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lv = thr_get_bit(thr, idx1);
	    unsigned rv = thr_get_bit(thr, idx2);

	    if ((lv < 2) && (rv < 2) && (lv != rv)) {
		  eq = 0;
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

      unsigned eq = (l == r)? 1 : 0;
      unsigned lt = (l <  r)? 1 : 0;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);

      return true;
}

bool of_CMPZ(vthread_t thr, vvp_code_t cp)
{
      unsigned eq = 1;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    unsigned lv = thr_get_bit(thr, idx1);
	    unsigned rv = thr_get_bit(thr, idx2);

	    if ((lv < 3) && (rv < 3) && (lv != rv)) {
		  eq = 0;
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
      thr->words[cp->bit_idx[0]].w_int = (long)(r);

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
	    thr_put_bit(thr, base+idx, (rl&1)? 1 : 0);
	    rl >>= 1;
      }

      return true;
}

bool of_DELAY(vthread_t thr, vvp_code_t cp)
{
	//printf("thread %p: %%delay %lu\n", thr, cp->number);
      schedule_vthread(thr, cp->number);
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
		  unsigned lb = thr_get_bit(thr, idx1);
		  unsigned rb = thr_get_bit(thr, idx2);

		  if ((lb | rb) & 2)
			goto x_out;

		  lv |= lb << idx;
		  rv |= rb << idx;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	    if (rv == 0)
		  goto x_out;

	    lv /= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? 1 : 0);
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
		  thr_put_bit(thr, cp->bit_idx[0]+idx, lbits[idx]);
	    }

	    delete[]lbits;
	    delete[]rbits;
	    return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? 1 : 0);
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
		  thr_put_bit(thr, cp->bit_idx[0]+idx, lbits[idx]);
	    }

	    delete[]lbits;
	    delete[]rbits;
      }

      return true;

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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

	/* If the new child was created to evaluate a function,
	   run it immediately, then return to this thread. */
      if (cp->scope->base.vpi_type->type_code == vpiFunction) {
	    child->is_scheduled = 1;
	    vthread_run(child);
      } else {
	    schedule_vthread(child, 0, true);
      }
      return true;
}

bool of_INV(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      for (unsigned idx = 0 ;  idx < cp->bit_idx[1] ;  idx += 1) {
	    unsigned val = thr_get_bit(thr, cp->bit_idx[0]+idx);
	    switch (val) {
		case 0:
		  val = 1;
		  break;
		case 1:
		  val = 0;
		  break;
		default:
		  val = 2;
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
      unsigned long v = 0;
      bool unknown_flag = false;

      for (unsigned i = 0; i<cp->number; i++) {
	    unsigned char vv = thr_get_bit(thr, cp->bit_idx[1] + i);
	    if (vv&2) {
		  v = 0UL;
		  unknown_flag = true;
		  break;
	    }
	    v |= vv << i;
      }
      thr->words[cp->bit_idx[0]].w_int = v;
	/* Set bit 4 as a flag if the input is unknown. */
      thr_put_bit(thr, 4, unknown_flag? 1 : 0);
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
      if (thr_get_bit(thr, cp->bit_idx[0]) != 1)
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

bool of_LOAD(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      thr_put_bit(thr, cp->bit_idx[0], functor_get(cp->iptr));
      return true;
}

bool of_LOAD_MEM(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      unsigned char val = memory_get(cp->mem, thr->words[3].w_int);
      thr_put_bit(thr, cp->bit_idx[0], val);
      return true;
}

/*
 * Load net/indexed.
 */
bool of_LOAD_NX(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      assert(cp->bit_idx[1] <  4);
      assert(cp->handle->vpi_type->type_code == vpiNet);

      struct __vpiSignal*sig =
	    reinterpret_cast<struct __vpiSignal*>(cp->handle);

      unsigned idx = thr->words[cp->bit_idx[1]].w_int;

      vvp_ipoint_t ptr = vvp_fvector_get(sig->bits, idx);
      thr_put_bit(thr, cp->bit_idx[0], functor_get(ptr));
      return true;
}

bool of_LOAD_VEC(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      assert(cp->bit_idx[1] > 0);

      unsigned bit = cp->bit_idx[0];
      for (unsigned idx = 0;  idx < cp->bit_idx[1];  idx += 1, bit += 1) {

	    vvp_ipoint_t iptr = ipoint_index(cp->iptr, idx);
	    thr_put_bit(thr, bit, functor_get(iptr));
      }

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

bool of_LOAD_X(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);
      assert(cp->bit_idx[1] <  4);

      vvp_ipoint_t ptr = ipoint_index(cp->iptr, thr->words[cp->bit_idx[1]].w_int);
      thr_put_bit(thr, cp->bit_idx[0], functor_get(ptr));
      return true;
}

bool of_LOADI_WR(vthread_t thr, vvp_code_t cp)
{
      unsigned idx = cp->bit_idx[0];
      double mant = cp->number;
      int exp = cp->bit_idx[1];
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
	    thr_put_bit(thr, cp->bit_idx[0]+idx, ob);
      }

      delete []t;
      delete []z;
      delete []a;
      return;

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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

		  lv |= lb << idx;
		  rv |= rb << idx;

		  idx1 += 1;
		  if (idx2 >= 4)
			idx2 += 1;
	    }

	    if (rv == 0)
		  goto x_out;

	    lv %= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? 1 : 0);
		  lv >>= 1;
	    }

	    return true;

      } else {
	    do_verylong_mod(thr, cp, false, false);
	    return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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

		  lv |= lb << idx;
		  rv |= rb << idx;

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
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? 1 : 0);
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
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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
static bool of_MOV0_a_(vthread_t thr, vvp_code_t cp)
{
      if ((cp->bit_idx[0]+cp->number) > thr->nbits)
	    thr_check_addr(thr, cp->bit_idx[0]+cp->number-1);

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_clr_bit_(thr, cp->bit_idx[0]+idx);

      return true;
}

static bool of_MOV0_b_(vthread_t thr, vvp_code_t cp)
{
      if (cp->bit_idx[1] >= thr->nbits)
	    thr_check_addr(thr, cp->bit_idx[1]);

      thr->bits[cp->bit_idx[0]] &= cp->number;
      return true;
}

static bool of_MOV1XZ_(vthread_t thr, vvp_code_t cp)
{
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, cp->bit_idx[1]);
      return true;
}

static bool of_MOV_(vthread_t thr, vvp_code_t cp)
{
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx,
			thr_get_bit(thr, cp->bit_idx[1]+idx));
      return true;
}

bool of_MOV(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->bit_idx[1] >= 4) {
	    cp->opcode = &of_MOV_;
	    return cp->opcode(thr, cp);

      } else if (cp->bit_idx[1] == 0) {
	      /* Detect the special case where this is really just a
		 large clear. Rewrite the instruction to skip this
		 test next time around, and use a precoded opcode. */

	    unsigned test_addr = cp->bit_idx[0] + cp->number - 1;

	    unsigned addr1 = cp->bit_idx[0] / (CPU_WORD_BITS/2);
	    unsigned addr2 = (test_addr) / (CPU_WORD_BITS/2);
	    if (addr1 == addr2) {
		  unsigned sh1 = cp->bit_idx[0] % (CPU_WORD_BITS/2);
		  unsigned sh2 = (test_addr % (CPU_WORD_BITS/2)) + 1;

		  unsigned long mask;

		  if ( (sh2-sh1) == CPU_WORD_BITS/2)
			mask = 0UL;
		  else
			mask = ULONG_MAX << ((sh2 - sh1) * 2UL);

		  mask = (~mask) << sh1*2UL;

		  cp->number = ~mask;
		  cp->bit_idx[0] = addr1;
		  cp->bit_idx[1] = test_addr;
		  cp->opcode = &of_MOV0_b_;
		  return cp->opcode(thr, cp);

	    } else {

		  cp->opcode = &of_MOV0_a_;
		  return cp->opcode(thr, cp);
	    }

      } else {
	    cp->opcode = &of_MOV1XZ_;
	    return cp->opcode(thr, cp);
      }

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
	    unsigned long lb = thr_get_bit(thr, idx1);
	    unsigned long rb = thr_get_bit(thr, idx2);

	    if ((lb | rb) & 2)
		  goto x_out;

	    lv |= lb << idx;
	    rv |= rb << idx;

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      lv *= rv;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? 1 : 0);
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
	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb | rb) & 2)
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
	    thr_put_bit(thr, cp->bit_idx[0]+idx, sum[idx]);
      }

      delete[]sum;
      delete[]b;
      delete[]a;
      return true;
      }

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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
		  unsigned lb = thr_get_bit(thr, idx1);

		  if (lb & 2)
			goto x_out;

		  lv |= lb << idx;

		  idx1 += 1;
	    }

	    lv *= rv;

	    for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
		  thr_put_bit(thr, cp->bit_idx[0]+idx, (lv&1) ? 1 : 0);
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
	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = imm & 1;

	    imm >>= 1;

	    if (lb & 2) {
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
	    thr_put_bit(thr, cp->bit_idx[0]+idx, sum[idx]);
      }

      delete[]sum;
      delete[]b;
      delete[]a;

      return true;

 x_out:
      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

      return true;
}

bool of_NAND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb == 0) || (rb == 0)) {
		  thr_put_bit(thr, idx1, 1);

	    } else if ((lb == 1) && (rb == 1)) {
		  thr_put_bit(thr, idx1, 0);

	    } else {
		  thr_put_bit(thr, idx1, 2);
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

      unsigned lb = 1;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned rb = thr_get_bit(thr, idx2+idx);
	    if (rb == 1) {
		  lb = 0;
		  break;
	    }

	    if (rb != 0)
		  lb = 2;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_ANDR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned lb = 1;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned rb = thr_get_bit(thr, idx2+idx);
	    if (rb == 0) {
		  lb = 0;
		  break;
	    }

	    if (rb != 1)
		  lb = 2;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_NANDR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned lb = 0;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned rb = thr_get_bit(thr, idx2+idx);
	    if (rb == 0) {
		  lb = 1;
		  break;
	    }

	    if (rb != 1)
		  lb = 2;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_ORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned lb = 0;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned rb = thr_get_bit(thr, idx2+idx);
	    if (rb == 1) {
		  lb = 1;
		  break;
	    }

	    if (rb != 0)
		  lb = 2;
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_XORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned lb = 0;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned rb = thr_get_bit(thr, idx2+idx);
	    if (rb == 1)
		  lb ^= 1;
	    else if (rb != 0) {
		  lb = 2;
		  break;
	    }
      }

      thr_put_bit(thr, cp->bit_idx[0], lb);

      return true;
}

bool of_XNORR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned lb = 1;
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned rb = thr_get_bit(thr, idx2+idx);
	    if (rb == 1)
		  lb ^= 1;
	    else if (rb != 0) {
		  lb = 2;
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

	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb == 1) || (rb == 1)) {
		  thr_put_bit(thr, idx1, 1);

	    } else if ((lb == 0) && (rb == 0)) {
		  thr_put_bit(thr, idx1, 0);

	    } else {
		  thr_put_bit(thr, idx1, 2);
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

	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb == 1) || (rb == 1)) {
		  thr_put_bit(thr, idx1, 0);

	    } else if ((lb == 0) && (rb == 0)) {
		  thr_put_bit(thr, idx1, 1);

	    } else {
		  thr_put_bit(thr, idx1, 2);
	    }

	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

static const unsigned char strong_values[4] = {St0, St1, StX, HiZ};

bool of_SET(vthread_t thr, vvp_code_t cp)
{
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[0]);
      functor_set(cp->iptr, bit_val, strong_values[bit_val], true);

      return true;
}

bool of_SET_MEM(vthread_t thr, vvp_code_t cp)
{
      unsigned char val = thr_get_bit(thr, cp->bit_idx[0]);
      memory_set(cp->mem, thr->words[3].w_int, val);

      return true;
}

bool of_SET_VEC(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[1] > 0);

      unsigned bit = cp->bit_idx[0];
      if (bit >= 4) {
	    for (unsigned idx = 0;  idx < cp->bit_idx[1]; idx += 1, bit += 1) {
		  unsigned char bit_val = thr_get_bit(thr, bit);
		  vvp_ipoint_t iptr = ipoint_index(cp->iptr, idx);
		  functor_set(iptr, bit_val, strong_values[bit_val], true);
	    }

      } else {
	    unsigned char bit_val = strong_values[bit];

	    for (unsigned idx = 0;  idx < cp->bit_idx[1]; idx += 1) {
		  vvp_ipoint_t iptr = ipoint_index(cp->iptr, idx);
		  functor_set(iptr, bit, bit_val, true);
	    }
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
 *      %set/x <functor>, <bit>, <idx>
 *
 * The single bit goes into the indexed functor. Abort the instruction
 * if the index is <0.
 */
bool of_SET_X0(vthread_t thr, vvp_code_t cp)
{
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[0]);
      long idx = thr->words[0].w_int;

	/* If idx < 0, then the index value is probably generated from
	   an undefined value. At any rate, this is defined to have no
	   effect so quit now. */
      if (idx < 0)
	    return true;

      if ((unsigned)idx > cp->bit_idx[1])
	    return true;

	/* Form the functor pointer from the base pointer and the
	   index from the index register. */
      vvp_ipoint_t itmp = ipoint_index(cp->iptr, idx);

	/* Set the value. */
      functor_set(itmp, bit_val, strong_values[bit_val], true);

      return true;
}

bool of_SET_X0_X(vthread_t thr, vvp_code_t cp)
{
      unsigned char bit_val = thr_get_bit(thr, cp->bit_idx[0]);
      long idx = thr->words[0].w_int;
      long lim = thr->words[cp->bit_idx[1]].w_int;

	/* If idx < 0, then the index value is probably generated from
	   an undefined value. At any rate, this is defined to have no
	   effect so quit now. */
      if (idx < 0)
	    return true;

      if (idx > lim)
	    return true;

	/* Form the functor pointer from the base pointer and the
	   index from the index register. */
      vvp_ipoint_t itmp = ipoint_index(cp->iptr, idx);

	/* Set the value. */
      functor_set(itmp, bit_val, strong_values[bit_val], true);

      return true;
}

bool of_SHIFTL_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;

      assert(base >= 4);

      if (shift >= wid) {
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, 0);

      } else if (shift > 0) {
	    for (unsigned idx = wid ;  idx > shift ;  idx -= 1) {
		  unsigned src = base+idx-shift-1;
		  unsigned dst = base + idx - 1;
		  thr_put_bit(thr, dst, thr_get_bit(thr, src));
	    }
	    for (unsigned idx = 0 ;  idx < shift ;  idx += 1)
		  thr_put_bit(thr, base+idx, 0);
      }
      return true;
}

/*
 * This is an unsigned right shift.
 */
bool of_SHIFTR_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;

      if (shift >= wid) {
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, 0);

      } else if (shift > 0) {
	    for (unsigned idx = 0 ;  idx < (wid-shift) ;  idx += 1) {
		  unsigned src = base + idx + shift;
		  unsigned dst = base + idx;
		  thr_put_bit(thr, dst, thr_get_bit(thr, src));
	    }
	    for (unsigned idx = (wid-shift) ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, 0);
      }
      return true;
}

bool of_SHIFTR_S_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;
      unsigned sign = thr_get_bit(thr, base+wid-1);

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
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (sum&1) ? 1 : 0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;
      delete[]lvb;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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
	    thr_put_bit(thr, cp->bit_idx[0]+idx, (bit&1) ? 1 : 0);
      }

      delete[]lva;
      delete[]lvb;

      return true;

 x_out:
      delete[]lva;

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+idx, 2);

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

/*
 * Implement the wait by locating the functor for the event, and
 * adding this thread to the threads list for the event.
 */
bool of_WAIT(vthread_t thr, vvp_code_t cp)
{
      assert(! thr->waiting_for_event);
      thr->waiting_for_event = 1;
      waitable_hooks_s* ep = dynamic_cast<waitable_hooks_s*>(functor_index(cp->iptr));
      assert(ep);
      thr->wait_next = ep->threads;
      ep->threads = thr;

      return false;
}


bool of_XNOR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {

	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb == 1) && (rb == 1)) {
		  thr_put_bit(thr, idx1, 1);

	    } else if ((lb == 0) && (rb == 0)) {
		  thr_put_bit(thr, idx1, 1);

	    } else if ((lb == 1) && (rb == 0)) {
		  thr_put_bit(thr, idx1, 0);

	    } else if ((lb == 0) && (rb == 1)) {
		  thr_put_bit(thr, idx1, 0);

	    } else {
		  thr_put_bit(thr, idx1, 2);
	    }

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

	    unsigned lb = thr_get_bit(thr, idx1);
	    unsigned rb = thr_get_bit(thr, idx2);

	    if ((lb == 1) && (rb == 1)) {
		  thr_put_bit(thr, idx1, 0);

	    } else if ((lb == 0) && (rb == 0)) {
		  thr_put_bit(thr, idx1, 0);

	    } else if ((lb == 1) && (rb == 0)) {
		  thr_put_bit(thr, idx1, 1);

	    } else if ((lb == 0) && (rb == 1)) {
		  thr_put_bit(thr, idx1, 1);

	    } else {
		  thr_put_bit(thr, idx1, 2);
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
 * Revision 1.122.2.1  2006/10/11 00:17:35  steve
 *  Get rounding of conversion correct.
 *
 * Revision 1.122  2004/10/04 01:11:00  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.121  2004/06/19 16:17:02  steve
 *  Watch type of mak bit matches masked value.
 *
 * Revision 1.120  2004/06/19 15:52:53  steve
 *  Add signed modulus operator.
 *
 * Revision 1.119  2004/06/04 23:26:34  steve
 *  Pick sign bit from the right place in the exponent number.
 *
 * Revision 1.118  2004/05/19 03:26:25  steve
 *  Support delayed/non-blocking assignment to reals and others.
 *
 * Revision 1.117  2003/11/10 20:19:32  steve
 *  Include config.h
 *
 * Revision 1.116  2003/09/26 02:15:15  steve
 *  Slight performance tweaks of scheduler.
 *
 * Revision 1.115  2003/09/01 04:03:38  steve
 *  32bit vs 64bit handling in SUBI.
 *
 * Revision 1.114  2003/08/01 00:58:03  steve
 *  Initialize allocated memory.
 *
 * Revision 1.113  2003/07/21 02:39:15  steve
 *  Overflow of unsigned when calculating unsigned long value.
 *
 * Revision 1.112  2003/07/03 20:03:36  steve
 *  Remove the vvp_cpoint_t indirect code pointer.
 *
 * Revision 1.111  2003/06/18 03:55:19  steve
 *  Add arithmetic shift operators.
 *
 * Revision 1.110  2003/06/17 21:28:59  steve
 *  Remove short int restrictions from vvp opcodes. (part 2)
 *
 * Revision 1.109  2003/06/17 19:17:42  steve
 *  Remove short int restrictions from vvp opcodes.
 *
 * Revision 1.108  2003/05/26 04:44:54  steve
 *  Add the set/x0/x instruction.
 *
 * Revision 1.107  2003/05/07 03:39:12  steve
 *  ufunc calls to functions can have scheduling complexities.
 *
 * Revision 1.106  2003/03/28 02:33:57  steve
 *  Add support for division of real operands.
 *
 * Revision 1.105  2003/03/13 04:36:57  steve
 *  Remove the obsolete functor delete functions.
 *
 * Revision 1.104  2003/02/27 20:36:29  steve
 *  Add the cvt/vr instruction.
 *
 * Revision 1.103  2003/02/22 06:26:58  steve
 *  When checking for stop, remember to reschedule.
 *
 * Revision 1.102  2003/02/22 02:52:06  steve
 *  Check for stopped flag in certain strategic points.
 *
 * Revision 1.101  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.100  2003/02/06 17:41:47  steve
 *  Add the %sub/wr instruction.
 */

