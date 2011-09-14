/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "vthread.h"
# include  "codes.h"
# include  "schedule.h"
# include  "ufunc.h"
# include  "event.h"
# include  "vpi_priv.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <typeinfo>
# include  <cstdlib>
# include  <climits>
# include  <cstring>
# include  <cmath>
# include  <cassert>

# include  <iostream>
# include  <cstdio>

/* This is the size of an unsigned long in bits. This is just a
   convenience macro. */
# define CPU_WORD_BITS (8*sizeof(unsigned long))
# define TOP_BIT (1UL << (CPU_WORD_BITS-1))

/*
 * This vthread_s structure describes all there is to know about a
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
      unsigned delay_delete      :1;
      unsigned fork_count        :8;
	/* This points to the sole child of the thread. */
      struct vthread_s*child;
	/* This points to my parent, if I have one. */
      struct vthread_s*parent;
	/* This is used for keeping wait queues. */
      struct vthread_s*wait_next;
	/* These are used to keep the thread in a scope. */
      struct vthread_s*scope_next, *scope_prev;
	/* These are used to access automatically allocated items. */
      vvp_context_t wt_context, rd_context;
	/* These are used to pass non-blocking event control information. */
      vvp_net_t*event;
      uint64_t ecount;
};

struct vthread_s*running_thread = 0;

// this table maps the thread special index bit addresses to
// vvp_bit4_t bit values.
static vvp_bit4_t thr_index_to_bit4[4] = { BIT4_0, BIT4_1, BIT4_X, BIT4_Z };

static inline void thr_check_addr(struct vthread_s*thr, unsigned addr)
{
      if (thr->bits4.size() <= addr)
	    thr->bits4.resize(addr+1);
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
      if (vpi_mode_flag == VPI_MODE_COMPILETF) return BIT4_X;
      else return thr_get_bit(thr, addr);
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
      if (addr == 0) {
	    unsigned awid = (wid + CPU_WORD_BITS - 1) / (CPU_WORD_BITS);
	    unsigned long*val = new unsigned long[awid];
	    for (unsigned idx = 0 ;  idx < awid ;  idx += 1)
		  val[idx] = 0;
	    return val;
      }
      if (addr == 1) {
	    unsigned awid = (wid + CPU_WORD_BITS - 1) / (CPU_WORD_BITS);
	    unsigned long*val = new unsigned long[awid];
	    for (unsigned idx = 0 ;  idx < awid ;  idx += 1)
		  val[idx] = -1UL;

	    wid -= (awid-1) * CPU_WORD_BITS;
	    if (wid < CPU_WORD_BITS)
		  val[awid-1] &= (-1UL) >> (CPU_WORD_BITS-wid);

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
	    return vvp_vector4_t(wid, thr_index_to_bit4[bit]);
      }
}

/*
 * Some of the instructions do wide addition to arrays of long. They
 * use this add_with_cary function to help.
 */
static inline unsigned long add_with_carry(unsigned long a, unsigned long b,
					   unsigned long&carry)
{
      unsigned long tmp = b + carry;
      unsigned long sum = a + tmp;
      carry = 0;
      if (tmp < b)
	    carry = 1;
      if (sum < tmp)
	    carry = 1;
      if (sum < a)
	    carry = 1;
      return sum;
}

static unsigned long multiply_with_carry(unsigned long a, unsigned long b,
					 unsigned long&carry)
{
      const unsigned long mask = (1UL << (CPU_WORD_BITS/2)) - 1;
      unsigned long a0 = a & mask;
      unsigned long a1 = (a >> (CPU_WORD_BITS/2)) & mask;
      unsigned long b0 = b & mask;
      unsigned long b1 = (b >> (CPU_WORD_BITS/2)) & mask;

      unsigned long tmp = a0 * b0;

      unsigned long r00 = tmp & mask;
      unsigned long c00 = (tmp >> (CPU_WORD_BITS/2)) & mask;

      tmp = a0 * b1;

      unsigned long r01 = tmp & mask;
      unsigned long c01 = (tmp >> (CPU_WORD_BITS/2)) & mask;

      tmp = a1 * b0;

      unsigned long r10 = tmp & mask;
      unsigned long c10 = (tmp >> (CPU_WORD_BITS/2)) & mask;

      tmp = a1 * b1;

      unsigned long r11 = tmp & mask;
      unsigned long c11 = (tmp >> (CPU_WORD_BITS/2)) & mask;

      unsigned long r1 = c00 + r01 + r10;
      unsigned long r2 = (r1 >> (CPU_WORD_BITS/2)) & mask;
      r1 &= mask;
      r2 += c01 + c10 + r11;
      unsigned long r3 = (r2 >> (CPU_WORD_BITS/2)) & mask;
      r2 &= mask;
      r3 += c11;
      r3 &= mask;

      carry = (r3 << (CPU_WORD_BITS/2)) + r2;
      return (r1 << (CPU_WORD_BITS/2)) + r00;
}

static void multiply_array_imm(unsigned long*res, unsigned long*val,
			       unsigned words, unsigned long imm)
{
      for (unsigned idx = 0 ; idx < words ; idx += 1)
	    res[idx] = 0;

      for (unsigned mul_idx = 0 ; mul_idx < words ; mul_idx += 1) {
	    unsigned long sum;
	    unsigned long tmp = multiply_with_carry(val[mul_idx], imm, sum);

	    unsigned long carry = 0;
	    res[mul_idx] = add_with_carry(res[mul_idx], tmp, carry);
	    for (unsigned add_idx = mul_idx+1 ; add_idx < words ; add_idx += 1) {
		  res[add_idx] = add_with_carry(res[add_idx], sum, carry);
		  sum = 0;
	    }
      }
}

/*
 * Allocate a context for use by a child thread. By preference, use
 * the last freed context. If none available, create a new one. Add
 * it to the list of live contexts in that scope.
 */
static vvp_context_t vthread_alloc_context(struct __vpiScope*scope)
{
      assert(scope->is_automatic);

      vvp_context_t context = scope->free_contexts;
      if (context) {
            scope->free_contexts = vvp_get_next_context(context);
            for (unsigned idx = 0 ; idx < scope->nitem ; idx += 1) {
                  scope->item[idx]->reset_instance(context);
            }
      } else {
            context = vvp_allocate_context(scope->nitem);
            for (unsigned idx = 0 ; idx < scope->nitem ; idx += 1) {
                  scope->item[idx]->alloc_instance(context);
            }
      }

      vvp_set_next_context(context, scope->live_contexts);
      scope->live_contexts = context;

      return context;
}

/*
 * Free a context previously allocated to a child thread by pushing it
 * onto the freed context stack. Remove it from the list of live contexts
 * in that scope.
 */
static void vthread_free_context(vvp_context_t context, struct __vpiScope*scope)
{
      assert(scope->is_automatic);
      assert(context);

      if (context == scope->live_contexts) {
            scope->live_contexts = vvp_get_next_context(context);
      } else {
            vvp_context_t tmp = scope->live_contexts;
            while (context != vvp_get_next_context(tmp)) {
                  assert(tmp);
                  tmp = vvp_get_next_context(tmp);
            }
            vvp_set_next_context(tmp, vvp_get_next_context(context));
      }

      vvp_set_next_context(context, scope->free_contexts);
      scope->free_contexts = context;
}

#ifdef CHECK_WITH_VALGRIND
void contexts_delete(struct __vpiScope*scope)
{
      vvp_context_t context = scope->free_contexts;

      while (context) {
	    scope->free_contexts = vvp_get_next_context(context);
	    for (unsigned idx = 0; idx < scope->nitem; idx += 1) {
		  scope->item[idx]->free_instance(context);
	    }
	    free(context);
	    context = scope->free_contexts;
      }
      free(scope->item);
}
#endif

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
      thr->wt_context = 0;
      thr->rd_context = 0;

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
      thr->delay_delete = 0;
      thr->waiting_for_event = 0;
      thr->fork_count   = 0;
      thr->event  = 0;
      thr->ecount = 0;

      thr_put_bit(thr, 0, BIT4_0);
      thr_put_bit(thr, 1, BIT4_1);
      thr_put_bit(thr, 2, BIT4_X);
      thr_put_bit(thr, 3, BIT4_Z);

      return thr;
}

#ifdef CHECK_WITH_VALGRIND
#if 0
/*
 * These are not currently correct. If you use them you will get
 * double delete messages. There is still a leak related to a
 * waiting event that needs to be investigated.
 */

static void wait_next_delete(vthread_t base)
{
      while (base) {
	    vthread_t tmp = base->wait_next;
	    delete base;
	    base = tmp;
	    if (base->waiting_for_event == 0) break;
      }
}

static void child_delete(vthread_t base)
{
      while (base) {
	    vthread_t tmp = base->child;
	    delete base;
	    base = tmp;
      }
}
#endif

void vthreads_delete(vthread_t base)
{
      if (base == 0) return;

      vthread_t cur = base->scope_next;
      while (base != cur) {
	    vthread_t tmp = cur->scope_next;
//	    if (cur->waiting_for_event) wait_next_delete(cur->wait_next);
//	    child_delete(cur->child);
	    delete cur;
	    cur = tmp;
      }
	/* This is a stub so does not have a wait_next queue. */
      delete base;
}
#endif

/*
 * Reaping pulls the thread out of the stack of threads. If I have a
 * child, then hand it over to my parent.
 */
static void vthread_reap(vthread_t thr)
{
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
	    if (thr->delay_delete)
		  schedule_del_thr(thr);
	    else
		  vthread_delete(thr);
      }
}

void vthread_delete(vthread_t thr)
{
      thr->bits4 = vvp_vector4_t();
      delete thr;
}

void vthread_mark_scheduled(vthread_t thr)
{
      while (thr != 0) {
	    assert(thr->is_scheduled == 0);
	    thr->is_scheduled = 1;
	    thr = thr->wait_next;
      }
}

void vthread_delay_delete()
{
      if (running_thread)
	    running_thread->delay_delete = 1;
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

            running_thread = thr;

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
      running_thread = 0;
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
 * The CHUNK_LINK instruction is a special next pointer for linking
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

vvp_context_t vthread_get_wt_context()
{
      if (running_thread)
            return running_thread->wt_context;
      else
            return 0;
}

vvp_context_t vthread_get_rd_context()
{
      if (running_thread)
            return running_thread->rd_context;
      else
            return 0;
}

vvp_context_item_t vthread_get_wt_context_item(unsigned context_idx)
{
      assert(running_thread && running_thread->wt_context);
      return vvp_get_context_item(running_thread->wt_context, context_idx);
}

vvp_context_item_t vthread_get_rd_context_item(unsigned context_idx)
{
      assert(running_thread && running_thread->rd_context);
      return vvp_get_context_item(running_thread->rd_context, context_idx);
}

bool of_ABS_WR(vthread_t thr, vvp_code_t cp)
{
      unsigned dst = cp->bit_idx[0];
      unsigned src = cp->bit_idx[1];

      thr->words[dst].w_real = fabs(thr->words[src].w_real);
      return true;
}

bool of_ALLOC(vthread_t thr, vvp_code_t cp)
{
        /* Allocate a context. */
      vvp_context_t child_context = vthread_alloc_context(cp->scope);

        /* Push the allocated context onto the write context stack. */
      vvp_set_stacked_context(child_context, thr->wt_context);
      thr->wt_context = child_context;

      return true;
}

static bool of_AND_wide(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      vvp_vector4_t val = vthread_bits_to_vector(thr, idx1, wid);
      val &= vthread_bits_to_vector(thr, idx2, wid);
      thr->bits4.set_vec(idx1, val);

      return true;
}

static bool of_AND_narrow(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);
	    thr_put_bit(thr, idx1, lb&rb);
	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

bool of_AND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->number <= 4)
	    cp->opcode = &of_AND_narrow;
      else
	    cp->opcode = &of_AND_wide;

      return cp->opcode(thr, cp);
}


bool of_ANDI(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned long imm = cp->bit_idx[1];
      unsigned wid = cp->number;

      assert(idx1 >= 4);

      vvp_vector4_t val = vthread_bits_to_vector(thr, idx1, wid);
      vvp_vector4_t imv (wid, BIT4_0);

      unsigned trans = wid;
      if (trans > CPU_WORD_BITS)
	    trans = CPU_WORD_BITS;
      imv.setarray(0, trans, &imm);

      val &= imv;

      thr->bits4.set_vec(idx1, val);
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
      for (unsigned idx = 0 ;  (idx*CPU_WORD_BITS) < cp->number ;  idx += 1)
	    lva[idx] = add_with_carry(lva[idx], lvb[idx], carry);

	/* We know from the vector_to_array that the address is valid
	   in the thr->bitr4 vector, so just do the set bit. */

      thr->bits4.setarray(cp->bit_idx[0], cp->number, lva);

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
      if (lva == 0)
	    goto x_out;


      unsigned long carry;
      carry = 0;
      for (unsigned idx = 0 ;  idx < word_count ;  idx += 1) {
	    lva[idx] = add_with_carry(lva[idx], imm_value, carry);
	    imm_value = 0;
      }

	/* We know from the vector_to_array that the address is valid
	   in the thr->bitr4 vector, so just do the set bit. */

      thr->bits4.setarray(bit_addr, bit_width, lva);

      delete[]lva;

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
      long off = thr->words[1].w_int;
      long adr = thr->words[3].w_int;
      unsigned delay = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];

      if (adr < 0) return true;

      long vwidth = get_array_word_size(cp->array);
	// We fell off the MSB end.
      if (off >= vwidth) return true;
	// Trim the bits after the MSB
      if (off + (long)wid > vwidth) {
	    wid += vwidth - off - wid;
      } else if (off < 0 ) {
	      // We fell off the LSB end.
	    if ((unsigned)-off > wid ) return true;
	      // Trim the bits before the LSB
	    wid += off;
	    bit -= off;
	    off = 0;
      }

      assert(wid > 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      schedule_assign_array_word(cp->array, adr, off, value, delay);
      return true;
}

/* %assign/av/d <array>, <delay_idx>, <bit>
 * This generates an assignment event to an array. Index register 0
 * contains the width of the vector (and the word) and index register
 * 3 contains the canonical address of the word in memory. The named
 * index register contains the delay.
 */
bool of_ASSIGN_AVD(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      long off = thr->words[1].w_int;
      long adr = thr->words[3].w_int;
      vvp_time64_t delay = thr->words[cp->bit_idx[0]].w_uint;
      unsigned bit = cp->bit_idx[1];

      if (adr < 0) return true;

      long vwidth = get_array_word_size(cp->array);
	// We fell off the MSB end.
      if (off >= vwidth) return true;
	// Trim the bits after the MSB
      if (off + (long)wid > vwidth) {
	    wid += vwidth - off - wid;
      } else if (off < 0 ) {
	      // We fell off the LSB end.
	    if ((unsigned)-off > wid ) return true;
	      // Trim the bits before the LSB
	    wid += off;
	    bit -= off;
	    off = 0;
      }

      assert(wid > 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      schedule_assign_array_word(cp->array, adr, off, value, delay);
      return true;
}

bool of_ASSIGN_AVE(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      long off = thr->words[1].w_int;
      long adr = thr->words[3].w_int;
      unsigned bit = cp->bit_idx[0];

      if (adr < 0) return true;

      long vwidth = get_array_word_size(cp->array);
	// We fell off the MSB end.
      if (off >= vwidth) return true;
	// Trim the bits after the MSB
      if (off + (long)wid > vwidth) {
	    wid += vwidth - off - wid;
      } else if (off < 0 ) {
	      // We fell off the LSB end.
	    if ((unsigned)-off > wid ) return true;
	      // Trim the bits before the LSB
	    wid += off;
	    bit -= off;
	    off = 0;
      }

      assert(wid > 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);
	// If the count is zero then just put the value.
      if (thr->ecount == 0) {
	    schedule_assign_array_word(cp->array, adr, off, value, 0);
      } else {
	    schedule_evctl(cp->array, adr, value, off, thr->event, thr->ecount);
      }
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

      vvp_net_ptr_t ptr (cp->net, 0);
      if (bit >= 4) {
	      // If the vector is not a synthetic one, then have the
	      // scheduler pluck it directly out of my vector space.
	    schedule_assign_plucked_vector(ptr, delay, thr->bits4, bit, wid);
      } else {
	    vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);
	    schedule_assign_plucked_vector(ptr, delay, value, 0, wid);
      }

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

      vvp_time64_t delay = thr->words[cp->bit_idx[0]].w_uint;
      unsigned bit = cp->bit_idx[1];

      vvp_net_ptr_t ptr (cp->net, 0);

      if (bit >= 4) {
	    schedule_assign_plucked_vector(ptr, delay, thr->bits4, bit, wid);
      } else {
	    vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);
	    schedule_assign_plucked_vector(ptr, delay, value, 0, wid);
      }

      return true;
}

/*
 * This is %assign/v0/e <label>, <bit>
 * Index register 0 contains a vector width.
 */
bool of_ASSIGN_V0E(vthread_t thr, vvp_code_t cp)
{
      assert(thr->event != 0);
      unsigned wid = thr->words[0].w_int;
      assert(wid > 0);
      unsigned bit = cp->bit_idx[0];

      vvp_net_ptr_t ptr (cp->net, 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);
	// If the count is zero then just put the value.
      if (thr->ecount == 0) {
	    schedule_assign_plucked_vector(ptr, 0, value, 0, wid);
      } else {
	    schedule_evctl(ptr, value, 0, 0, thr->event, thr->ecount);
      }

      thr->event = 0;
      thr->ecount = 0;

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
      long off = thr->words[1].w_int;
      unsigned delay = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];

      vvp_fun_signal_vec*sig
	    = reinterpret_cast<vvp_fun_signal_vec*> (cp->net->fun);
      assert(sig);

	// We fell off the MSB end.
      if (off >= (long)sig->size()) return true;
      else if (off < 0 ) {
	      // We fell off the LSB end.
	    if ((unsigned)-off >= wid ) return true;
	      // Trim the bits before the LSB
	    wid += off;
	    bit -= off;
	    off = 0;
      }

      assert(wid > 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
      schedule_assign_vector(ptr, off, sig->size(), value, delay);

      return true;
}

/*
 * This is %assign/v0/x1/d <label>, <delayx>, <bit>
 * Index register 0 contains a vector part width.
 * Index register 1 contains the offset into the destination vector.
 */
bool of_ASSIGN_V0X1D(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      long off = thr->words[1].w_int;
      vvp_time64_t delay = thr->words[cp->bit_idx[0]].w_uint;
      unsigned bit = cp->bit_idx[1];

      vvp_fun_signal_vec*sig
	    = reinterpret_cast<vvp_fun_signal_vec*> (cp->net->fun);
      assert(sig);

	// We fell off the MSB end.
      if (off >= (long)sig->size()) return true;
      else if (off < 0 ) {
	      // We fell off the LSB end.
	    if ((unsigned)-off >= wid ) return true;
	      // Trim the bits before the LSB
	    wid += off;
	    bit -= off;
	    off = 0;
      }

      assert(wid > 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
      schedule_assign_vector(ptr, off, sig->size(), value, delay);

      return true;
}

/*
 * This is %assign/v0/x1/e <label>, <bit>
 * Index register 0 contains a vector part width.
 * Index register 1 contains the offset into the destination vector.
 */
bool of_ASSIGN_V0X1E(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = thr->words[0].w_int;
      long off = thr->words[1].w_int;
      unsigned bit = cp->bit_idx[0];

      vvp_fun_signal_vec*sig
	    = reinterpret_cast<vvp_fun_signal_vec*> (cp->net->fun);
      assert(sig);

	// We fell off the MSB end.
      if (off >= (long)sig->size()) {
	    thr->event = 0;
	    thr->ecount = 0;
	    return true;
      } else if (off < 0 ) {
	      // We fell off the LSB end.
	    if ((unsigned)-off >= wid ) {
		  thr->event = 0;
		  thr->ecount = 0;
		  return true;
	    }
	      // Trim the bits before the LSB
	    wid += off;
	    bit -= off;
	    off = 0;
      }

      assert(wid > 0);

      vvp_vector4_t value = vthread_bits_to_vector(thr, bit, wid);

      vvp_net_ptr_t ptr (cp->net, 0);
	// If the count is zero then just put the value.
      if (thr->ecount == 0) {
	    schedule_assign_vector(ptr, off, sig->size(), value, 0);
      } else {
	    schedule_evctl(ptr, value, off, sig->size(), thr->event,
	                   thr->ecount);
      }

      thr->event = 0;
      thr->ecount = 0;

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
      vpi_put_value(tmp, &val, &del, vpiTransportDelay);

      return true;
}

bool of_ASSIGN_WRD(vthread_t thr, vvp_code_t cp)
{
      vvp_time64_t delay = thr->words[cp->bit_idx[0]].w_uint;
      unsigned index = cp->bit_idx[1];
      s_vpi_time del;

      del.type = vpiSimTime;
      vpip_time_to_timestruct(&del, delay);

      struct __vpiHandle*tmp = cp->handle;

      t_vpi_value val;
      val.format = vpiRealVal;
      val.value.real = thr->words[index].w_real;
      vpi_put_value(tmp, &val, &del, vpiTransportDelay);

      return true;
}

bool of_ASSIGN_WRE(vthread_t thr, vvp_code_t cp)
{
      assert(thr->event != 0);
      unsigned index = cp->bit_idx[0];
      struct __vpiHandle*tmp = cp->handle;

	// If the count is zero then just put the value.
      if (thr->ecount == 0) {
	    t_vpi_value val;

	    val.format = vpiRealVal;
	    val.value.real = thr->words[index].w_real;
	    vpi_put_value(tmp, &val, 0, vpiNoDelay);
      } else {
	    schedule_evctl(tmp, thr->words[index].w_real, thr->event,
	                   thr->ecount);
      }

      thr->event = 0;
      thr->ecount = 0;

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

bool of_BLEND_WR(vthread_t thr, vvp_code_t cp)
{
      double t = thr->words[cp->bit_idx[0]].w_real;
      double f = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = (t == f) ? t : 0.0;
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
	    vvp_net_ptr_t tmp (dst, 1);
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
      vvp_send_vec4(ptr, value, 0);

      return true;
}

bool of_CASSIGN_WR(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net  = cp->net;
      double value = thr->words[cp->bit_idx[0]].w_real;

	/* Set the value into port 1 of the destination. */
      vvp_net_ptr_t ptr (net, 1);
      vvp_send_real(ptr, value, 0);

      return true;
}

bool of_CASSIGN_X0(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned base = cp->bit_idx[0];
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

      vvp_vector4_t vector = vthread_bits_to_vector(thr, base, wid);

      vvp_net_ptr_t ptr (net, 1);
      vvp_send_vec4_pv(ptr, vector, index, wid, sig->size(), 0);

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

/*
 * The of_CMPIU below punts to this function if there are any xz bits
 * in the vector part of the instruction. In this case we know that
 * there is at least 1 xz bit in the left expression (and there are
 * none in the imm value) so the eeq result must be false. Otherwise,
 * the eq result may me 0 or x, and the lt bit is x.
 */
static bool of_CMPIU_the_hard_way(vthread_t thr, vvp_code_t cp)
{

      unsigned idx1 = cp->bit_idx[0];
      unsigned long imm  = cp->bit_idx[1];
      unsigned wid  = cp->number;
      if (idx1 >= 4)
	    thr_check_addr(thr, idx1+wid-1);

      vvp_bit4_t lv = thr_get_bit(thr, idx1);
      vvp_bit4_t eq  = BIT4_1;
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    vvp_bit4_t rv = (imm & 1UL)? BIT4_1 : BIT4_0;
	    imm >>= 1UL;

	    if (bit4_is_xz(lv)) {
		  eq = BIT4_X;
	    } else if (lv != rv) {
		  eq = BIT4_0;
		  break;
	    }

	    if (idx1 >= 4) {
		  idx1 += 1;
		  if ((idx+1) < wid)
			lv = thr_get_bit(thr, idx1);
	    }
      }

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, BIT4_X);
      thr_put_bit(thr, 6, BIT4_0);

      return true;
}

bool of_CMPIU(vthread_t thr, vvp_code_t cp)
{
      unsigned addr = cp->bit_idx[0];
      unsigned long imm  = cp->bit_idx[1];
      unsigned wid  = cp->number;

      unsigned long*array = vector_to_array(thr, addr, wid);
	// If there are xz bits in the right hand expression, then we
	// have to do the compare the hard way. That is because even
	// though we know that eeq must be false (the immediate value
	// cannot have x or z bits) we don't know what the EQ or LT
	// bits will be.
      if (array == 0)
	    return of_CMPIU_the_hard_way(thr, cp);

      unsigned words = (wid+CPU_WORD_BITS-1) / CPU_WORD_BITS;
      vvp_bit4_t eq  = BIT4_1;
      vvp_bit4_t lt  = BIT4_0;
      for (unsigned idx = 0 ; idx < words ; idx += 1, imm = 0UL) {
	    if (array[idx] == imm)
		  continue;

	    eq = BIT4_0;
	    lt = (array[idx] < imm) ? BIT4_1 : BIT4_0;
      }

      delete[]array;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eq);
      return true;
}

bool of_CMPU_the_hard_way(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq = BIT4_1;
      vvp_bit4_t eeq = BIT4_1;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];

      for (unsigned idx = 0 ;  idx < cp->number ;  idx += 1) {
	    vvp_bit4_t lv = thr_get_bit(thr, idx1);
	    vvp_bit4_t rv = thr_get_bit(thr, idx2);

	    if (lv != rv)
		  eeq = BIT4_0;

	    if (eq==BIT4_1 && (bit4_is_xz(lv) || bit4_is_xz(rv)))
		  eq = BIT4_X;
	    if ((lv == BIT4_0) && (rv==BIT4_1))
		  eq = BIT4_0;
	    if ((lv == BIT4_1) && (rv==BIT4_0))
		  eq = BIT4_0;

	    if (eq == BIT4_0)
		  break;

	    if (idx1 >= 4) idx1 += 1;
	    if (idx2 >= 4) idx2 += 1;

      }

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, BIT4_X);
      thr_put_bit(thr, 6, eeq);

      return true;
}

bool of_CMPU(vthread_t thr, vvp_code_t cp)
{
      vvp_bit4_t eq = BIT4_1;
      vvp_bit4_t lt = BIT4_0;

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid  = cp->number;

      unsigned long*larray = vector_to_array(thr, idx1, wid);
      if (larray == 0) return of_CMPU_the_hard_way(thr, cp);

      unsigned long*rarray = vector_to_array(thr, idx2, wid);
      if (rarray == 0) {
	    delete[]larray;
	    return of_CMPU_the_hard_way(thr, cp);
      }

      unsigned words = (wid+CPU_WORD_BITS-1) / CPU_WORD_BITS;

      for (unsigned wdx = 0 ; wdx < words ; wdx += 1) {
	    if (larray[wdx] == rarray[wdx])
		  continue;

	    eq = BIT4_0;
	    if (larray[wdx] < rarray[wdx])
		  lt = BIT4_1;
	    else
		  lt = BIT4_0;
      }

      delete[]larray;
      delete[]rarray;

      thr_put_bit(thr, 4, eq);
      thr_put_bit(thr, 5, lt);
      thr_put_bit(thr, 6, eq);

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
      thr->words[cp->bit_idx[0]].w_int = i64round(r);

      return true;
}

bool of_CVT_RI(vthread_t thr, vvp_code_t cp)
{
      int64_t r = thr->words[cp->bit_idx[1]].w_int;
      thr->words[cp->bit_idx[0]].w_real = (double)(r);

      return true;
}

bool of_CVT_VR(vthread_t thr, vvp_code_t cp)
{
      double r = thr->words[cp->bit_idx[1]].w_real;
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      vvp_vector4_t tmp(wid, r);
	/* Make sure there is enough space for the new vector. */
      thr_check_addr(thr, base+wid-1);
      thr->bits4.set_vec(base, tmp);

      return true;
}

/*
 * This implements the %deassign instruction. All we do is write a
 * long(1) to port-3 of the addressed net. This turns off an active
 * continuous assign activated by %cassign/v
 */
bool of_DEASSIGN(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned base  = cp->bit_idx[0];
      unsigned width = cp->bit_idx[1];

      vvp_fun_signal_vec*sig = reinterpret_cast<vvp_fun_signal_vec*>(net->fun);
      assert(sig);

      if (base >= sig->size()) return true;
      if (base+width > sig->size()) width = sig->size() - base;

      bool full_sig = base == 0 && width == sig->size();

	// This is the net that is forcing me...
      if (vvp_net_t*src = sig->cassign_link) {
	    if (!full_sig) {
		  fprintf(stderr, "Sorry: when a signal is assigning a "
		          "register, I cannot deassign part of it.\n");
		  exit(1);
	    }
	      // And this is the pointer to be removed.
	    vvp_net_ptr_t dst_ptr (net, 1);
	    unlink_from_driver(src, dst_ptr);
	    sig->cassign_link = 0;
      }

	/* Do we release all or part of the net? */
      vvp_net_ptr_t ptr (net, 3);
      if (full_sig) {
	    vvp_send_long(ptr, 1);
      } else {
	    vvp_send_long_pv(ptr, 1, base, width);
      }

      return true;
}

bool of_DEASSIGN_WR(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;

      vvp_fun_signal_real*sig = reinterpret_cast<vvp_fun_signal_real*>(net->fun);
      assert(sig);

	// This is the net that is forcing me...
      if (vvp_net_t*src = sig->cassign_link) {
	      // And this is the pointer to be removed.
	    vvp_net_ptr_t dst_ptr (net, 1);
	    unlink_from_driver(src, dst_ptr);
	    sig->cassign_link = 0;
      }

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
      vvp_time64_t delay;

      assert(cp->number < 4);
      delay = thr->words[cp->number].w_uint;
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

/*
 * This function divides a 2-word number {high, a} by a 1-word
 * number. Assume that high < b.
 */
static unsigned long divide2words(unsigned long a, unsigned long b,
				  unsigned long high)
{
      unsigned long result = 0;
      while (high > 0) {
	    unsigned long tmp_result = ULONG_MAX / b;
	    unsigned long remain = ULONG_MAX % b;

	    remain += 1;
	    if (remain >= b) {
		  remain -= b;
		  tmp_result += 1;
	    }

	      // Now 0x1_0...0 = b*tmp_result + remain
	      // high*0x1_0...0 = high*(b*tmp_result + remain)
	      // high*0x1_0...0 = high*b*tmp_result + high*remain

	      // We know that high*0x1_0...0 >= high*b*tmp_result, and
	      // we know that high*0x1_0...0 > high*remain. Use
	      // high*remain as the remainder for another iteration,
	      // and add tmp_result*high into the current estimate of
	      // the result.
	    result += tmp_result * high;

	      // The new iteration starts with high*remain + a.
	    remain = multiply_with_carry(high, remain, high);
	    a += remain;
            if(a < remain)
              high += 1;

	      // Now result*b + {high,a} == the input {high,a}. It is
	      // possible that the new high >= 1. If so, it will
	      // certainly be less than high from the previous
	      // iteration. Do another iteration and it will shrink,
	      // eventually to 0.
      }

	// high is now 0, so a is the remaining remainder, so we can
	// finish off the integer divide with a simple a/b.

      return result + a/b;
}

static unsigned long* divide_bits(unsigned long*ap, unsigned long*bp, unsigned wid)
{
	// Do all our work a cpu-word at a time. The "words" variable
	// is the number of words of the wid.
      unsigned words = (wid+CPU_WORD_BITS-1) / CPU_WORD_BITS;

      unsigned btop = words-1;
      while (btop > 0 && bp[btop] == 0)
	    btop -= 1;

	// Detect divide by 0, and exit.
      if (btop==0 && bp[0]==0)
	    return 0;

	// The result array will eventually accumulate the result. The
	// diff array is a difference that we use in the intermediate.
      unsigned long*diff  = new unsigned long[words];
      unsigned long*result= new unsigned long[words];
      for (unsigned idx = 0 ; idx < words ; idx += 1)
	    result[idx] = 0;

      for (unsigned cur = words-btop ; cur > 0 ; cur -= 1) {
	    unsigned cur_ptr = cur-1;
	    unsigned long cur_res;
	    if (ap[cur_ptr+btop] >= bp[btop]) {
		  unsigned long high = 0;
		  if (cur_ptr+btop+1 < words)
			high = ap[cur_ptr+btop+1];
		  cur_res = divide2words(ap[cur_ptr+btop], bp[btop], high);

	    } else if (cur_ptr+btop+1 >= words) {
		  continue;

	    } else if (ap[cur_ptr+btop+1] == 0) {
		  continue;

	    } else {
		  cur_res = divide2words(ap[cur_ptr+btop], bp[btop],
					 ap[cur_ptr+btop+1]);
	    }

	      // cur_res is a guesstimate of the result this far. It
	      // may be 1 too big. (But it will also be >0) Try it,
	      // and if the difference comes out negative, then adjust.

	      // diff = (bp * cur_res)  << cur_ptr;
	    multiply_array_imm(diff+cur_ptr, bp, words-cur_ptr, cur_res);
	      // ap -= diff
	    unsigned long carry = 1;
	    for (unsigned idx = cur_ptr ; idx < words ; idx += 1)
		  ap[idx] = add_with_carry(ap[idx], ~diff[idx], carry);

	      // ap has the diff subtracted out of it. If cur_res was
	      // too large, then ap will turn negative. (We easily
	      // tell that ap turned negative by looking at
	      // carry&1. If it is 0, then it is *negative*.) In that
	      // case, we know that cur_res was too large by 1. Correct by
	      // adding 1b back in and reducing cur_res.
	    if ((carry&1) == 0) {
		    // Keep adding b back in until the remainder
		    // becomes positive again.
		  do {
			cur_res -= 1;
			carry = 0;
			for (unsigned idx = cur_ptr ; idx < words ; idx += 1)
			      ap[idx] = add_with_carry(ap[idx], bp[idx-cur_ptr], carry);
		  } while (carry == 0);
	    }

	    result[cur_ptr] = cur_res;
      }

	// Now ap contains the remainder and result contains the
	// desired result. We should find that:
	//  input-a = bp * result + ap;

      delete[]diff;
      return result;
}

bool of_DIV(vthread_t thr, vvp_code_t cp)
{
      unsigned adra = cp->bit_idx[0];
      unsigned adrb = cp->bit_idx[1];
      unsigned wid = cp->number;

      assert(adra >= 4);

      unsigned long*ap = vector_to_array(thr, adra, wid);
      if (ap == 0) {
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

      unsigned long*bp = vector_to_array(thr, adrb, wid);
      if (bp == 0) {
	    delete[]ap;
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

	// If the value fits in a single CPU word, then do it the easy way.
      if (wid <= CPU_WORD_BITS) {
	    if (bp[0] == 0) {
		  vvp_vector4_t tmp(wid, BIT4_X);
		  thr->bits4.set_vec(adra, tmp);
	    } else {
		  ap[0] /= bp[0];
		  thr->bits4.setarray(adra, wid, ap);
	    }
	    delete[]ap;
	    delete[]bp;
	    return true;
      }

      unsigned long*result = divide_bits(ap, bp, wid);
      if (result == 0) {
	    delete[]ap;
	    delete[]bp;
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

	// Now ap contains the remainder and result contains the
	// desired result. We should find that:
	//  input-a = bp * result + ap;

      thr->bits4.setarray(adra, wid, result);
      delete[]ap;
      delete[]bp;
      delete[]result;
      return true;
}


static void negate_words(unsigned long*val, unsigned words)
{
      unsigned long carry = 1;
      for (unsigned idx = 0 ; idx < words ; idx += 1)
	    val[idx] = add_with_carry(0, ~val[idx], carry);
}

bool of_DIV_S(vthread_t thr, vvp_code_t cp)
{
      unsigned adra = cp->bit_idx[0];
      unsigned adrb = cp->bit_idx[1];
      unsigned wid = cp->number;
      unsigned words = (wid + CPU_WORD_BITS - 1) / CPU_WORD_BITS;

      assert(adra >= 4);

	// Get the values, left in right, in binary form. If there is
	// a problem with either (caused by an X or Z bit) then we
	// know right away that the entire result is X.
      unsigned long*ap = vector_to_array(thr, adra, wid);
      if (ap == 0) {
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

      unsigned long*bp = vector_to_array(thr, adrb, wid);
      if (bp == 0) {
	    delete[]ap;
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

	// Sign extend the bits in the array to fill out the array.
      unsigned long sign_mask = 0;
      if (unsigned long sign_bits = (words*CPU_WORD_BITS) - wid) {
	    sign_mask = -1UL << (CPU_WORD_BITS-sign_bits);
	    if (ap[words-1] & (sign_mask>>1))
		  ap[words-1] |= sign_mask;
	    if (bp[words-1] & (sign_mask>>1))
		  bp[words-1] |= sign_mask;
      }

	// If the value fits in a single word, then use the native divide.
      if (wid <= CPU_WORD_BITS) {
	    if (bp[0] == 0) {
		  vvp_vector4_t tmp(wid, BIT4_X);
		  thr->bits4.set_vec(adra, tmp);
	    } else {
		  long tmpa = (long) ap[0];
		  long tmpb = (long) bp[0];
		  long res = tmpa / tmpb;
		  ap[0] = ((unsigned long)res) & ~sign_mask;
		  thr->bits4.setarray(adra, wid, ap);
	    }
	    delete[]ap;
	    delete[]bp;
	    return true;
      }

	// We need to the actual division to positive integers. Make
	// them positive here, and remember the negations.
      bool negate_flag = false;
      if ( ((long) ap[words-1]) < 0 ) {
	    negate_flag = true;
	    negate_words(ap, words);
      }
      if ( ((long) bp[words-1]) < 0 ) {
	    negate_flag ^= true;
	    negate_words(bp, words);
      }

      unsigned long*result = divide_bits(ap, bp, wid);
      if (result == 0) {
	    delete[]ap;
	    delete[]bp;
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

      if (negate_flag) {
	    negate_words(result, words);
      }

      result[words-1] &= ~sign_mask;

      thr->bits4.setarray(adra, wid, result);
      delete[]ap;
      delete[]bp;
      delete[]result;
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

bool of_EVCTL(vthread_t thr, vvp_code_t cp)
{
      assert(thr->event == 0 && thr->ecount == 0);
      thr->event = cp->net;
      thr->ecount = thr->words[cp->bit_idx[0]].w_uint;
      return true;
}
bool of_EVCTLC(vthread_t thr, vvp_code_t)
{
      thr->event = 0;
      thr->ecount = 0;
      return true;
}

bool of_EVCTLI(vthread_t thr, vvp_code_t cp)
{
      assert(thr->event == 0 && thr->ecount == 0);
      thr->event = cp->net;
      thr->ecount = cp->bit_idx[0];
      return true;
}

bool of_EVCTLS(vthread_t thr, vvp_code_t cp)
{
      assert(thr->event == 0 && thr->ecount == 0);
      thr->event = cp->net;
      int64_t val = thr->words[cp->bit_idx[0]].w_int;
      if (val < 0) val = 0;
      thr->ecount = val;
      return true;
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

	/* Set the value into port 2 of the destination. */
      vvp_net_ptr_t ptr (net, 2);
      vvp_send_vec4(ptr, value, 0);

      return true;
}

bool of_FORCE_WR(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net  = cp->net;
      double value = thr->words[cp->bit_idx[0]].w_real;

	/* Set the value into port 2 of the destination. */
      vvp_net_ptr_t ptr (net, 2);
      vvp_send_real(ptr, value, 0);

      return true;
}


bool of_FORCE_X0(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned base = cp->bit_idx[0];
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

      vvp_vector4_t vector = vthread_bits_to_vector(thr, base, wid);

      vvp_net_ptr_t ptr (net, 2);
      vvp_send_vec4_pv(ptr, vector, index, wid, sig->size(), 0);

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
      if (cp->scope->is_automatic) {
              /* The context allocated for this child is the top entry
                 on the write context stack. */
            child->wt_context = thr->wt_context;
            child->rd_context = thr->wt_context;
      }

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
            running_thread = thr;
      } else {
	    schedule_vthread(child, 0, true);
      }

      return true;
}

bool of_FREE(vthread_t thr, vvp_code_t cp)
{
        /* Pop the child context from the read context stack. */
      vvp_context_t child_context = thr->rd_context;
      thr->rd_context = vvp_get_stacked_context(child_context);

        /* Free the context. */
      vthread_free_context(child_context, cp->scope);

      return true;
}

static bool of_INV_wide(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];

      vvp_vector4_t val = vthread_bits_to_vector(thr, idx1, wid);
      thr->bits4.set_vec(idx1, ~val);

      return true;
}

static bool of_INV_narrow(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    thr_put_bit(thr, idx1, ~lb);
	    idx1 += 1;
      }

      return true;
}

bool of_INV(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->number <= 4)
	    cp->opcode = &of_INV_narrow;
      else
	    cp->opcode = &of_INV_wide;

      return cp->opcode(thr, cp);
}


/*
 * Index registers, arithmetic.
 */

static inline int64_t get_as_64_bit(uint32_t low_32, uint32_t high_32)
{
      int64_t low = low_32;
      int64_t res = high_32;

      res <<= 32;
      res |= low;
      return res;
}

bool of_IX_ADD(vthread_t thr, vvp_code_t cp)
{
      thr->words[cp->number].w_int += get_as_64_bit(cp->bit_idx[0],
                                                    cp->bit_idx[1]);
      return true;
}

bool of_IX_SUB(vthread_t thr, vvp_code_t cp)
{
      thr->words[cp->number].w_int -= get_as_64_bit(cp->bit_idx[0],
                                                    cp->bit_idx[1]);
      return true;
}

bool of_IX_MUL(vthread_t thr, vvp_code_t cp)
{
      thr->words[cp->number].w_int *= get_as_64_bit(cp->bit_idx[0],
                                                    cp->bit_idx[1]);
      return true;
}

bool of_IX_LOAD(vthread_t thr, vvp_code_t cp)
{
      thr->words[cp->number].w_int = get_as_64_bit(cp->bit_idx[0],
                                                   cp->bit_idx[1]);
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

static uint64_t vector_to_index(vthread_t thr, unsigned base,
                                unsigned width, bool signed_flag)
{
      uint64_t v = 0;
      bool unknown_flag = false;

      vvp_bit4_t vv = BIT4_0;
      for (unsigned i = 0 ;  i < width ;  i += 1) {
	    vv = thr_get_bit(thr, base);
	    if (bit4_is_xz(vv)) {
		  v = 0UL;
		  unknown_flag = true;
		  break;
	    }

	    v |= (uint64_t) vv << i;

	    if (base >= 4)
		  base += 1;
      }

	/* Extend to fill the integer value. */
      if (signed_flag && !unknown_flag) {
	    uint64_t pad = vv;
	    for (unsigned i = width ; i < 8*sizeof(v) ;  i += 1) {
		  v |= pad << i;
	    }
      }

	/* Set bit 4 as a flag if the input is unknown. */
      thr_put_bit(thr, 4, unknown_flag ? BIT4_1 : BIT4_0);

      return v;
}

bool of_IX_GET(vthread_t thr, vvp_code_t cp)
{
      unsigned index = cp->bit_idx[0];
      unsigned base  = cp->bit_idx[1];
      unsigned width = cp->number;

      thr->words[index].w_uint = vector_to_index(thr, base, width, false);
      return true;
}

bool of_IX_GET_S(vthread_t thr, vvp_code_t cp)
{
      unsigned index = cp->bit_idx[0];
      unsigned base  = cp->bit_idx[1];
      unsigned width = cp->number;

      thr->words[index].w_int = vector_to_index(thr, base, width, true);
      return true;
}

bool of_IX_GETV(vthread_t thr, vvp_code_t cp)
{
      unsigned index = cp->bit_idx[0];
      vvp_net_t*net = cp->net;

      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*>(net->fun);
      if (sig == 0) {
	    cerr << "%%ix/getv error: Net arg not a vector signal? "
		 << typeid(*net->fun).name() << endl;
      }
      assert(sig);

      vvp_vector4_t vec = sig->vec4_value();
      uint64_t val;
      bool known_flag = vector4_to_value(vec, val);

      if (known_flag)
	    thr->words[index].w_uint = val;
      else
	    thr->words[index].w_uint = 0;

	/* Set bit 4 as a flag if the input is unknown. */
      thr_put_bit(thr, 4, known_flag ? BIT4_0 : BIT4_1);

      return true;
}

bool of_IX_GETV_S(vthread_t thr, vvp_code_t cp)
{
      unsigned index = cp->bit_idx[0];
      vvp_net_t*net = cp->net;

      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*>(net->fun);
      if (sig == 0) {
	    cerr << "%%ix/getv/s error: Net arg not a vector signal? "
		 << typeid(*net->fun).name() << endl;
      }
      assert(sig);

      vvp_vector4_t vec = sig->vec4_value();
      int64_t val;
      bool known_flag = vector4_to_value(vec, val, true, true);

      if (known_flag)
	    thr->words[index].w_int = val;
      else
	    thr->words[index].w_int = 0;

	/* Set bit 4 as a flag if the input is unknown. */
      thr_put_bit(thr, 4, known_flag ? BIT4_0 : BIT4_1);

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

        /* If the child thread is in an automatic scope... */
      if (thr->child->wt_context) {
              /* and is the top level task/function thread... */
            if (thr->wt_context != thr->rd_context) {
                    /* Pop the child context from the write context stack. */
                  vvp_context_t child_context = thr->wt_context;
                  thr->wt_context = vvp_get_stacked_context(child_context);

                    /* Push the child context onto the read context stack */
                  vvp_set_stacked_context(child_context, thr->rd_context);
                  thr->rd_context = child_context;
            }
      }

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
 * %load/ar <bit>, <array-label>, <index>;
*/
bool of_LOAD_AR(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned idx = cp->bit_idx[1];
      unsigned adr = thr->words[idx].w_int;
      double word;

	/* The result is 0.0 if the address is undefined. */
      if (thr_get_bit(thr, 4) == BIT4_1) {
	    word = 0.0;
      } else {
	    word = array_get_word_r(cp->array, adr);
      }

      thr->words[bit].w_real = word;
      return true;
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

	/* Check the address once, before we scan the vector. */
      thr_check_addr(thr, bit+wid-1);

	/* The result is 'bx if the address is undefined. */
      if (thr_get_bit(thr, 4) == BIT4_1) {
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(bit, tmp);
	    return true;
      }

      vvp_vector4_t word = array_get_word(cp->array, adr);

      if (word.size() > wid)
	    word.resize(wid);

	/* Copy the vector bits into the bits4 vector. Do the copy
	   directly to skip the excess calls to thr_check_addr. */
      thr->bits4.set_vec(bit, word);

	/* If the source is shorter then the desired width, then pad
	   with BIT4_X values. */
      for (unsigned idx = word.size() ; idx < wid ; idx += 1)
	    thr->bits4.set_bit(bit+idx, BIT4_X);

      return true;
}

/*
 * %load/vp0, %load/vp0/s, %load/avp0 and %load/avp0/s share this function.
*/
#if (SIZEOF_UNSIGNED_LONG >= 8)
# define CPU_WORD_STRIDE CPU_WORD_BITS - 1  // avoid a warning
#else
# define CPU_WORD_STRIDE CPU_WORD_BITS
#endif
static void load_vp0_common(vthread_t thr, vvp_code_t cp, const vvp_vector4_t&sig_value)
{
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];
      int64_t addend = thr->words[0].w_int;

	/* Check the address once, before we scan the vector. */
      thr_check_addr(thr, bit+wid-1);

      unsigned long*val = sig_value.subarray(0, wid);
      if (val == 0) {
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(bit, tmp);
	    return;
      }

      unsigned words = (wid + CPU_WORD_BITS - 1) / CPU_WORD_BITS;
      unsigned long carry = 0;
      unsigned long imm = addend;
      for (unsigned idx = 0 ; idx < words ; idx += 1) {
            val[idx] = add_with_carry(val[idx], imm, carry);
            addend >>= CPU_WORD_STRIDE;
            imm = addend;
      }

	/* Copy the vector bits into the bits4 vector. Do the copy
	   directly to skip the excess calls to thr_check_addr. */
      thr->bits4.setarray(bit, wid, val);
      delete[]val;
}

/*
 * %load/avp0 <bit>, <array-label>, <wid> ;
 *
 * <bit> is the thread bit address for the result
 * <array-label> is the array to access, and
 * <wid> is the width of the word to read.
 *
 * The address of the word in the array is in index register 3.
 * An integer value from index register 0 is added to the value.
 */
bool of_LOAD_AVP0(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = cp->bit_idx[1];
      unsigned adr = thr->words[3].w_int;

	/* The result is 'bx if the address is undefined. */
      if (thr_get_bit(thr, 4) == BIT4_1) {
	    unsigned bit = cp->bit_idx[0];
	    thr_check_addr(thr, bit+wid-1);
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(bit, tmp);
	    return true;
      }

        /* We need a vector this wide to make the math work correctly.
         * Copy the base bits into the vector, but keep the width. */
      vvp_vector4_t sig_value(wid, BIT4_0);
      sig_value.copy_bits(array_get_word(cp->array, adr));

      load_vp0_common(thr, cp, sig_value);
      return true;
}

bool of_LOAD_AVP0_S(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = cp->bit_idx[1];
      unsigned adr = thr->words[3].w_int;

	/* The result is 'bx if the address is undefined. */
      if (thr_get_bit(thr, 4) == BIT4_1) {
	    unsigned bit = cp->bit_idx[0];
	    thr_check_addr(thr, bit+wid-1);
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(bit, tmp);
	    return true;
      }

      vvp_vector4_t tmp (array_get_word(cp->array, adr));

        /* We need a vector this wide to make the math work correctly.
         * Copy the base bits into the vector, but keep the width. */
      vvp_vector4_t sig_value(wid, tmp.value(tmp.size()-1));
      sig_value.copy_bits(tmp);

      load_vp0_common(thr, cp, sig_value);
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

	/* The result is 'bx if the address is undefined. */
      if (thr_get_bit(thr, 4) == BIT4_1) {
	    thr_put_bit(thr, bit, BIT4_X);
	    return true;
      }

      long use_index = thr->words[index].w_int;

      vvp_vector4_t word = array_get_word(cp->array, adr);

      if ((use_index >= (long)word.size()) || (use_index < 0)) {
	    thr_put_bit(thr, bit, BIT4_X);
      } else {
	    thr_put_bit(thr, bit, word.value(use_index));
      }

      thr->words[index].w_int = use_index + 1;

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
static vvp_vector4_t load_base(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;

	/* For the %load to work, the functor must actually be a
	   signal functor. Only signals save their vector value. */
      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (net->fun);
      if (sig == 0) {
	    cerr << "%%load/v error: Net arg not a vector signal? "
		 << typeid(*net->fun).name() << endl;
	    assert(sig);
      }

      return sig->vec4_value();
}

bool of_LOAD_VEC(vthread_t thr, vvp_code_t cp)
{
      unsigned bit = cp->bit_idx[0];
      unsigned wid = cp->bit_idx[1];

      vvp_vector4_t sig_value = load_base(thr, cp);

	/* Check the address once, before we scan the vector. */
      thr_check_addr(thr, bit+wid-1);

      if (sig_value.size() > wid)
	    sig_value.resize(wid);

	/* Copy the vector bits into the bits4 vector. Do the copy
	   directly to skip the excess calls to thr_check_addr. */
      thr->bits4.set_vec(bit, sig_value);

	/* If the source is shorter then the desired width, then pad
	   with BIT4_X values. */
      for (unsigned idx = sig_value.size() ; idx < wid ; idx += 1)
	    thr->bits4.set_bit(bit+idx, BIT4_X);

      return true;
}

/*
 * This is like of_LOAD_VEC, but includes an add of an integer value from
 * index 0. The <wid> is the expected result width not the vector width.
 */

bool of_LOAD_VP0(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = cp->bit_idx[1];

        /* We need a vector this wide to make the math work correctly.
         * Copy the base bits into the vector, but keep the width. */
      vvp_vector4_t sig_value(wid, BIT4_0);
      sig_value.copy_bits(load_base(thr, cp));

      load_vp0_common(thr, cp, sig_value);
      return true;
}

bool of_LOAD_VP0_S(vthread_t thr, vvp_code_t cp)
{
      unsigned wid = cp->bit_idx[1];

      vvp_vector4_t tmp (load_base(thr, cp));

        /* We need a vector this wide to make the math work correctly.
         * Copy the base bits into the vector, but keep the width. */
      vvp_vector4_t sig_value(wid, tmp.value(tmp.size()-1));
      sig_value.copy_bits(tmp);

      load_vp0_common(thr, cp, sig_value);
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
 * %load/x16 <bit>, <functor>, <wid>
 *
 * <bit> is the destination thread bit and must be >= 4.
 */
bool of_LOAD_X1P(vthread_t thr, vvp_code_t cp)
{
	// <bit> is the thread bit to load
      assert(cp->bit_idx[0] >= 4);
      unsigned bit = cp->bit_idx[0];
      int wid = cp->bit_idx[1];

	// <index> is the canonical base address of the part select.
      long index = thr->words[1].w_int;

	// <functor> is converted to a vvp_net_t pointer from which we
	// read our value.
      vvp_net_t*net = cp->net;

	// For the %load to work, the functor must actually be a
	// signal functor. Only signals save their vector value.
      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (net->fun);
      assert(sig);

      for (long idx = 0 ; idx < wid ; idx += 1) {
	    long use_index = index + idx;
	    vvp_bit4_t val;
	    if (use_index < 0 || use_index >= (signed)sig->size())
		  val = BIT4_X;
	    else
		  val = sig->value(use_index);

	    thr_put_bit(thr, bit+idx, val);
      }

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
      if (exp==0x3fff) {
	    thr->words[idx].w_real = nan("");
	    return true;
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
      bool out_is_neg = left_is_neg;
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

	      /* Sign extend the signed operands when needed. */
	    if (cp->number < 8*sizeof(long long)) {
		  if (lv & (1LL << (cp->number-1)))
			lv |= -1LL << cp->number;
		  if (rv & (1LL << (cp->number-1)))
			rv |= -1LL << cp->number;
	    }

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
      vvp_vector4_t tmp (cp->number, thr_index_to_bit4[cp->bit_idx[1]]);
      thr->bits4.set_vec(cp->bit_idx[0], tmp);
      return true;
}

static bool of_MOV_(vthread_t thr, vvp_code_t cp)
{
	/* This variant implements the general case that we know
	   neither the source nor the destination to be <4. Otherwise,
	   we copy all the bits manually. */

      thr_check_addr(thr, cp->bit_idx[0]+cp->number-1);
      thr_check_addr(thr, cp->bit_idx[1]+cp->number-1);
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
      static unsigned long val[8] = {0, 0, 0, 0, 0, 0, 0, 0};
      unsigned wid = cp->number;

      thr_check_addr(thr, dst+wid-1);

      val[0] = cp->bit_idx[1];

      while (wid > 0) {
	    unsigned trans = wid;
	    if (trans > 8*CPU_WORD_BITS)
		  trans = 8*CPU_WORD_BITS;

	    thr->bits4.setarray(dst, trans, val);

	    val[0] = 0;
	    wid -= trans;
	    dst += trans;
      }

      return true;
}

bool of_MUL(vthread_t thr, vvp_code_t cp)
{
      unsigned adra = cp->bit_idx[0];
      unsigned adrb = cp->bit_idx[1];
      unsigned wid = cp->number;

      assert(adra >= 4);

      unsigned long*ap = vector_to_array(thr, adra, wid);
      if (ap == 0) {
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

      unsigned long*bp = vector_to_array(thr, adrb, wid);
      if (bp == 0) {
	    delete[]ap;
	    vvp_vector4_t tmp(wid, BIT4_X);
	    thr->bits4.set_vec(adra, tmp);
	    return true;
      }

	// If the value fits in a single CPU word, then do it the easy way.
      if (wid <= CPU_WORD_BITS) {
	    ap[0] *= bp[0];
	    thr->bits4.setarray(adra, wid, ap);
	    delete[]ap;
	    delete[]bp;
	    return true;
      }

      unsigned words = (wid+CPU_WORD_BITS-1) / CPU_WORD_BITS;
      unsigned long*res = new unsigned long[words];
      for (unsigned idx = 0 ; idx < words ; idx += 1)
	    res[idx] = 0;

      for (unsigned mul_a = 0 ; mul_a < words ; mul_a += 1) {
	    for (unsigned mul_b = 0 ; mul_b < (words-mul_a) ; mul_b += 1) {
		  unsigned long sum;
		  unsigned long tmp = multiply_with_carry(ap[mul_a], bp[mul_b], sum);
		  unsigned base = mul_a + mul_b;
		  unsigned long carry = 0;
		  res[base] = add_with_carry(res[base], tmp, carry);
		  for (unsigned add_idx = base+1; add_idx < words; add_idx += 1) {
			res[add_idx] = add_with_carry(res[add_idx], sum, carry);
			sum = 0;
		  }
	    }
      }

      thr->bits4.setarray(adra, wid, res);
      delete[]ap;
      delete[]bp;
      delete[]res;
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
      unsigned adr = cp->bit_idx[0];
      unsigned long imm = cp->bit_idx[1];
      unsigned wid = cp->number;

      assert(adr >= 4);

      unsigned long*val = vector_to_array(thr, adr, wid);
	// If there are X bits in the value, then return X.
      if (val == 0) {
	    vvp_vector4_t tmp(cp->number, BIT4_X);
	    thr->bits4.set_vec(cp->bit_idx[0], tmp);
	    return true;
      }

	// If everything fits in a word, then do it the easy way.
      if (wid <= CPU_WORD_BITS) {
	    val[0] *= imm;
	    thr->bits4.setarray(adr, wid, val);
	    delete[]val;
	    return true;
      }

      unsigned words = (wid+CPU_WORD_BITS-1) / CPU_WORD_BITS;
      unsigned long*res = new unsigned long[words];

      multiply_array_imm(res, val, words, imm);

      thr->bits4.setarray(adr, wid, res);
      delete[]val;
      delete[]res;
      return true;
}

static bool of_NAND_wide(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      vvp_vector4_t val = vthread_bits_to_vector(thr, idx1, wid);
      val &= vthread_bits_to_vector(thr, idx2, wid);
      thr->bits4.set_vec(idx1, ~val);

      return true;
}

static bool of_NAND_narrow(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);
	    thr_put_bit(thr, idx1, ~(lb&rb));
	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

bool of_NAND(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->number <= 4)
	    cp->opcode = &of_NAND_narrow;
      else
	    cp->opcode = &of_NAND_wide;

      return cp->opcode(thr, cp);
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

static bool of_OR_wide(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      vvp_vector4_t val = vthread_bits_to_vector(thr, idx1, wid);
      val |= vthread_bits_to_vector(thr, idx2, wid);
      thr->bits4.set_vec(idx1, val);

      return true;
}

static bool of_OR_narrow(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);
	    thr_put_bit(thr, idx1, lb|rb);
	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

bool of_OR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->number <= 4)
	    cp->opcode = &of_OR_narrow;
      else
	    cp->opcode = &of_OR_wide;

      return cp->opcode(thr, cp);
}

static bool of_NOR_wide(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      vvp_vector4_t val = vthread_bits_to_vector(thr, idx1, wid);
      val |= vthread_bits_to_vector(thr, idx2, wid);
      thr->bits4.set_vec(idx1, ~val);

      return true;
}

static bool of_NOR_narrow(vthread_t thr, vvp_code_t cp)
{
      unsigned idx1 = cp->bit_idx[0];
      unsigned idx2 = cp->bit_idx[1];
      unsigned wid = cp->number;

      for (unsigned idx = 0 ; idx < wid ; idx += 1) {
	    vvp_bit4_t lb = thr_get_bit(thr, idx1);
	    vvp_bit4_t rb = thr_get_bit(thr, idx2);
	    thr_put_bit(thr, idx1, ~(lb|rb));
	    idx1 += 1;
	    if (idx2 >= 4)
		  idx2 += 1;
      }

      return true;
}

bool of_NOR(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      if (cp->number <= 4)
	    cp->opcode = &of_NOR_narrow;
      else
	    cp->opcode = &of_NOR_wide;

      return cp->opcode(thr, cp);
}

bool of_POW(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx = cp->bit_idx[0];
      unsigned idy = cp->bit_idx[1];
      unsigned wid = cp->number;
      vvp_vector2_t xv2 = vvp_vector2_t(vthread_bits_to_vector(thr, idx, wid));
      vvp_vector2_t yv2 = vvp_vector2_t(vthread_bits_to_vector(thr, idy, wid));

        /* If we have an X or Z in the arguments return X. */
      if (xv2.is_NaN() || yv2.is_NaN()) {
	    for (unsigned jdx = 0 ;  jdx < wid ;  jdx += 1)
		  thr_put_bit(thr, cp->bit_idx[0]+jdx, BIT4_X);
	    return true;
      }

        /* To make the result more manageable trim off the extra bits. */
      xv2.trim();
      yv2.trim();

      vvp_vector2_t result = pow(xv2, yv2);

        /* If the result is too small zero pad it. */
      if (result.size() < wid) {
	    for (unsigned jdx = wid-1;  jdx >= result.size();  jdx -= 1)
		  thr_put_bit(thr, cp->bit_idx[0]+jdx, BIT4_0);
	    wid = result.size();
      }

        /* Copy only what we need of the result. */
      for (unsigned jdx = 0;  jdx < wid;  jdx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+jdx,
	                result.value(jdx) ? BIT4_1 : BIT4_0);

      return true;
}

bool of_POW_S(vthread_t thr, vvp_code_t cp)
{
      assert(cp->bit_idx[0] >= 4);

      unsigned idx = cp->bit_idx[0];
      unsigned idy = cp->bit_idx[1];
      unsigned wid = cp->number;
      vvp_vector4_t xv = vthread_bits_to_vector(thr, idx, wid);
      vvp_vector4_t yv = vthread_bits_to_vector(thr, idy, wid);

        /* If we have an X or Z in the arguments return X. */
      if (xv.has_xz() || yv.has_xz()) {
	    for (unsigned jdx = 0 ;  jdx < wid ;  jdx += 1)
		  thr_put_bit(thr, cp->bit_idx[0]+jdx, BIT4_X);
	    return true;
      }

        /* Calculate the result using the double pow() function. */
      double xd, yd, resd;
      vector4_to_value(xv, xd, true);
      vector4_to_value(yv, yd, true);
	/* 2**-1 and -2**-1 are defined to be zero. */
      if ((yd == -1.0) && (fabs(xd) == 2.0)) resd = 0.0;
      else resd = pow(xd, yd);
      vvp_vector4_t res = vvp_vector4_t(wid, resd);

        /* Copy the result. */
      for (unsigned jdx = 0;  jdx < wid;  jdx += 1)
	    thr_put_bit(thr, cp->bit_idx[0]+jdx, res.value(jdx));

      return true;
}

bool of_POW_WR(vthread_t thr, vvp_code_t cp)
{
      double l = thr->words[cp->bit_idx[0]].w_real;
      double r = thr->words[cp->bit_idx[1]].w_real;
      thr->words[cp->bit_idx[0]].w_real = pow(l, r);

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
      unsigned base  = cp->bit_idx[0];
      unsigned width = cp->bit_idx[1];

      vvp_fun_signal_vec*sig = reinterpret_cast<vvp_fun_signal_vec*>(net->fun);
      assert(sig);

      if (base >= sig->size()) return true;
      if (base+width > sig->size()) width = sig->size() - base;

      bool full_sig = base == 0 && width == sig->size();

      if (sig->force_link) {
	    if (!full_sig) {
		  fprintf(stderr, "Sorry: when a signal is forcing a "
		          "net, I cannot release part of it.\n");
		  exit(1);
	    }
	    unlink_force(net);
      }
      assert(sig->force_link == 0);

	/* Do we release all or part of the net? */
      vvp_net_ptr_t ptr (net, 3);
      if (full_sig) {
	    vvp_send_long(ptr, 2);
      } else {
	    vvp_send_long_pv(ptr, 2, base, width);
      }

      return true;
}


bool of_RELEASE_REG(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned base  = cp->bit_idx[0];
      unsigned width = cp->bit_idx[1];

      vvp_fun_signal_vec*sig = reinterpret_cast<vvp_fun_signal_vec*>(net->fun);
      assert(sig);

      if (base >= sig->size()) return true;
      if (base+width > sig->size()) width = sig->size() - base;

      bool full_sig = base == 0 && width == sig->size();

	// This is the net that is forcing me...
      if (vvp_net_t*src = sig->force_link) {
	    if (!full_sig) {
		  fprintf(stderr, "Sorry: when a signal is forcing a "
		          "register, I cannot release part of it.\n");
		  exit(1);
	    }
	      // And this is the pointer to be removed.
	    vvp_net_ptr_t dst_ptr (net, 2);
	    unlink_from_driver(src, dst_ptr);
	    sig->force_link = 0;
      }

	// Send a command to this signal to unforce itself.
	/* Do we release all or part of the net? */
      vvp_net_ptr_t ptr (net, 3);
      if (full_sig) {
	    vvp_send_long(ptr, 3);
      } else {
	    vvp_send_long_pv(ptr, 3, base, width);
      }

      return true;
}

/* The type is 1 for registers and 0 for everything else. */
bool of_RELEASE_WR(vthread_t thr, vvp_code_t cp)
{
      vvp_net_t*net = cp->net;
      unsigned type  = cp->bit_idx[0];

      vvp_fun_signal_real*sig = reinterpret_cast<vvp_fun_signal_real*>(net->fun);
      assert(sig);

	// This is the net that is forcing me...
      if (vvp_net_t*src = sig->force_link) {
	      // And this is the pointer to be removed.
	    vvp_net_ptr_t dst_ptr (net, 2);
	    unlink_from_driver(src, dst_ptr);
	    sig->force_link = 0;
      }

	// Send a command to this signal to unforce itself.
      vvp_net_ptr_t ptr (net, 3);
      vvp_send_long(ptr, 2 + type);

      return true;
}

/*
 * %set/av <label>, <index>, <bit>
 *
 * Write the real value in register <bit> to the array indexed by the
 * integer value addressed bin index register <index>.
 */
bool of_SET_AR(vthread_t thr, vvp_code_t cp)
{
      unsigned idx = cp->bit_idx[0];
      unsigned bit = cp->bit_idx[1];
      unsigned adr = thr->words[idx].w_int;

      double value = thr->words[bit].w_real;
      array_set_word(cp->array, adr, value);

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

      vvp_send_vec4(ptr, vthread_bits_to_vector(thr, bit, wid),
                    thr->wt_context);

      return true;
}

bool of_SET_WORDR(vthread_t thr, vvp_code_t cp)
{
	/* set the value into port 0 of the destination. */
      vvp_net_ptr_t ptr (cp->net, 0);

      vvp_send_real(ptr, thr->words[cp->bit_idx[0]].w_real, thr->wt_context);

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

	// If the entire part is below the beginning of the vector,
	// then we are done.
      if (index < 0 && (wid <= (unsigned)-index))
	    return true;

	// If the entire part is above then end of the vector, then we
	// are done.
      if (index >= (long)sig->size())
	    return true;

	// If the part starts below the vector, then skip the first
	// few bits and reduce enough bits to start at the beginning
	// of the vector.
      if (index < 0) {
	    if (bit >= 4) bit += (unsigned) -index;
	    wid -= (unsigned) -index;
	    index = 0;
      }

	// Reduce the width to keep the part inside the vector.
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
      vvp_send_vec4_pv(ptr, bit_vec, index, wid, sig->size(), thr->wt_context);

      return true;
}

bool of_SHIFTL_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      long shift = thr->words[0].w_int;

      assert(base >= 4);
      thr_check_addr(thr, base+wid-1);

      if (thr_get_bit(thr, 4) == BIT4_1) {
	    // The result is 'bx if the shift amount is undefined.
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(base, tmp);

      } else if (shift >= (long)wid) {
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

      } else if (shift < 0) {
	      // For a negative shift we pad with 'bx.
	    unsigned idx;
	    for (idx = 0 ;  (idx-shift) < wid ;  idx += 1) {
		  unsigned src = base + idx - shift;
		  unsigned dst = base + idx;
		  thr_put_bit(thr, dst, thr_get_bit(thr, src));
	    }
	    for ( ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, BIT4_X);
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
      long shift = thr->words[0].w_int;

      assert(base >= 4);
      thr_check_addr(thr, base+wid-1);

      if (thr_get_bit(thr, 4) == BIT4_1) {
	      // The result is 'bx if the shift amount is undefined.
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(base, tmp);

      } else if (shift > 0) {
	    unsigned idx;
	    for (idx = 0 ;  (idx+shift) < wid ;  idx += 1) {
		  unsigned src = base + idx + shift;
		  unsigned dst = base + idx;
		  thr_put_bit(thr, dst, thr_get_bit(thr, src));
	    }
	    for ( ;  idx < wid ;  idx += 1)
		  thr_put_bit(thr, base+idx, BIT4_0);

      } else if (shift < -(long)wid) {
	      // Negative shift is so far that all the value is shifted out.
	      // Write in a constant 'bx result.
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(base, tmp);

      } else if (shift < 0) {

	      // For a negative shift we pad with 'bx.
	    vvp_vector4_t tmp (thr->bits4, base, wid+shift);
	    thr->bits4.set_vec(base-shift, tmp);

	    vvp_vector4_t fil (-shift, BIT4_X);
	    thr->bits4.set_vec(base, fil);
      }
      return true;
}

bool of_SHIFTR_S_I0(vthread_t thr, vvp_code_t cp)
{
      unsigned base = cp->bit_idx[0];
      unsigned wid = cp->number;
      unsigned long shift = thr->words[0].w_int;
      vvp_bit4_t sign = thr_get_bit(thr, base+wid-1);

      if (thr_get_bit(thr, 4) == BIT4_1) {
	      // The result is 'bx if the shift amount is undefined.
	    vvp_vector4_t tmp (wid, BIT4_X);
	    thr->bits4.set_vec(base, tmp);
      } else if (shift >= wid) {
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


      unsigned long carry;
      carry = 1;
      for (unsigned idx = 0 ;  (idx*CPU_WORD_BITS) < cp->number ;  idx += 1)
	    lva[idx] = add_with_carry(lva[idx], ~lvb[idx], carry);


	/* We know from the vector_to_array that the address is valid
	   in the thr->bitr4 vector, so just do the set bit. */

      thr->bits4.setarray(cp->bit_idx[0], cp->number, lva);
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
      unsigned long imm = cp->bit_idx[1];
      unsigned long*lva = vector_to_array(thr, cp->bit_idx[0], cp->number);
      if (lva == 0)
	    goto x_out;


      unsigned long carry;
      carry = 1;
      for (unsigned idx = 0 ;  idx < word_count ;  idx += 1) {
	    lva[idx] = add_with_carry(lva[idx], ~imm, carry);
	    imm = 0UL;
      }

	/* We know from the vector_to_array that the address is valid
	   in the thr->bitr4 vector, so just do the set bit. */

      thr->bits4.setarray(cp->bit_idx[0], cp->number, lva);

      delete[]lva;

      return true;

 x_out:
      delete[]lva;

      vvp_vector4_t tmp(cp->number, BIT4_X);
      thr->bits4.set_vec(cp->bit_idx[0], tmp);

      return true;
}

bool of_VPI_CALL(vthread_t thr, vvp_code_t cp)
{
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

	/* Add this thread to the list in the event. */
      waitable_hooks_s*ep = dynamic_cast<waitable_hooks_s*> (cp->net->fun);
      assert(ep);
      thr->wait_next = ep->add_waiting_thread(thr);

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
      if ((thr->parent == 0) && (thr->child == 0)) {
	    if (thr->delay_delete)
		  schedule_del_thr(thr);
	    else
		  vthread_delete(thr);
      }
      return false;
}

/*
 * This is a phantom opcode used to call user defined functions. It
 * is used in code generated by the .ufunc statement. It contains a
 * pointer to the executable code of the function and a pointer to
 * a ufunc_core object that has all the port information about the
 * function.
 */
bool of_EXEC_UFUNC(vthread_t thr, vvp_code_t cp)
{
      struct __vpiScope*child_scope = cp->ufunc_core_ptr->func_scope();
      assert(child_scope);

      assert(thr->child == 0);
      assert(thr->fork_count == 0);

        /* We can take a number of shortcuts because we know that a
           continuous assignment can only occur in a static scope. */
      assert(thr->wt_context == 0);
      assert(thr->rd_context == 0);

        /* If an automatic function, allocate a context for this call. */
      vvp_context_t child_context = 0;
      if (child_scope->is_automatic) {
            child_context = vthread_alloc_context(child_scope);
            thr->wt_context = child_context;
            thr->rd_context = child_context;
      }
	/* Copy all the inputs to the ufunc object to the port
	   variables of the function. This copies all the values
	   atomically. */
      cp->ufunc_core_ptr->assign_bits_to_ports(child_context);

	/* Create a temporary thread and run it immediately. A function
           may not contain any blocking statements, so vthread_run() can
           only return when the %end opcode is reached. */
      vthread_t child = vthread_new(cp->cptr, child_scope);
      child->wt_context = child_context;
      child->rd_context = child_context;
      child->is_scheduled = 1;
      vthread_run(child);
      running_thread = thr;

	/* Now copy the output from the result variable to the output
	   ports of the .ufunc device. */
      cp->ufunc_core_ptr->finish_thread(thr);

        /* If an automatic function, free the context for this call. */
      if (child_scope->is_automatic) {
            vthread_free_context(child_context, child_scope);
            thr->wt_context = 0;
            thr->rd_context = 0;
      }

      return true;
}
