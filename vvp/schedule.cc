/*
 * Copyright (c) 2001-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "config.h"
# include  "schedule.h"
# include  "vthread.h"
# include  "vpi_priv.h"
# include  "vvp_net_sig.h"
# include  "slab.h"
# include  "compile.h"
# include  <new>
# include  <typeinfo>
# include  <csignal>
# include  <cstdlib>
# include  <cassert>
# include  <iostream>
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
# include  "ivl_alloc.h"
#endif

using namespace std;

unsigned long count_assign_events = 0;
unsigned long count_gen_events = 0;
unsigned long count_thread_events = 0;
  // Count the time events (A time cell created)
unsigned long count_time_events = 0;



/*
 * The event_s and event_time_s structures implement the Verilog
 * stratified event queue.
 *
 * The event_time_s objects are one per time step. Each time step in
 * turn contains a list of event_s objects that are the actual events.
 *
 * The event_s objects are base classes for the more specific sort of
 * event.
 */
struct event_s {
      struct event_s*next;
      virtual ~event_s() { }
      virtual void run_run(void) =0;

	// Write something about the event to stderr
      virtual void single_step_display(void);

	// Fallback new/delete
      static void*operator new (size_t size) { return ::new char[size]; }
      static void operator delete(void*ptr)  { ::delete[]( (char*)ptr ); }
};

void event_s::single_step_display(void)
{
      std::cerr << "event_s: Step into event " << typeid(*this).name() << std::endl;
}

struct event_time_s {
      event_time_s() {
	    count_time_events += 1;
	    delay = 0;
	    start = 0;
	    active = 0;
	    inactive = 0;
	    nbassign = 0;
	    rwsync = 0;
	    rosync = 0;
	    del_thr = 0;
	    next = NULL;
      }
      vvp_time64_t delay;

      struct event_s*start;
      struct event_s*active;
      struct event_s*inactive;
      struct event_s*nbassign;
      struct event_s*rwsync;
      struct event_s*rosync;
      struct event_s*del_thr;

      struct event_time_s*next;

      static void* operator new (size_t);
      static void operator delete(void*obj, size_t s);
};

vvp_gen_event_s::~vvp_gen_event_s()
{
}

void vvp_gen_event_s::single_step_display(void)
{
      cerr << "vvp_gen_event_s: Step into event " << typeid(*this).name() << endl;
}

/*
 * Derived event types
 */
struct vthread_event_s : public event_s {
      vthread_t thr;
      void run_run(void);
      void single_step_display(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void vthread_event_s::run_run(void)
{
      count_thread_events += 1;
      vthread_run(thr);
}

void vthread_event_s::single_step_display(void)
{
      __vpiScope*scope = vthread_scope(thr);
      cerr << "vthread_event: Resume thread"
	   << " scope=" << scope->vpi_get_str(vpiFullName)
	   << endl;
}

static const size_t VTHR_CHUNK_COUNT = 8192 / sizeof(struct vthread_event_s);
static slab_t<sizeof(vthread_event_s),VTHR_CHUNK_COUNT> vthread_event_heap;

inline void* vthread_event_s::operator new(size_t size)
{
      assert(size == sizeof(vthread_event_s));
      return vthread_event_heap.alloc_slab();
}

void vthread_event_s::operator delete(void*dptr)
{
      vthread_event_heap.free_slab(dptr);
}

struct del_thr_event_s : public event_s {
      vthread_t thr;
      void run_run(void);
      void single_step_display(void);
};

void del_thr_event_s::run_run(void)
{
      vthread_delete(thr);
}

void del_thr_event_s::single_step_display(void)
{
      __vpiScope*scope = vthread_scope(thr);
      cerr << "del_thr_event: Reap completed thread"
	   << " scope=" << scope->vpi_get_str(vpiFullName) << endl;
}

struct assign_vector4_event_s  : public event_s {
	/* The default constructor. */
      explicit assign_vector4_event_s(const vvp_vector4_t&that) : val(that) {
	    base = 0;
	    vwid = 0;
      }

	/* Where to do the assign. */
      vvp_net_ptr_t ptr;
	/* Value to assign. */
      vvp_vector4_t val;
	/* Offset of the part into the destination. */
      unsigned base;
	/* Width of the destination vector. */
      unsigned vwid;
      void run_run(void);
      void single_step_display(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_vector4_event_s::run_run(void)
{
      count_assign_events += 1;
      if (vwid > 0)
	    vvp_send_vec4_pv(ptr, val, base, vwid, 0);
      else
	    vvp_send_vec4(ptr, val, 0);
}

void assign_vector4_event_s::single_step_display(void)
{
      cerr << "assign_vector4_event: Propagate val=" << val
	   << ", vwid=" << vwid << ", base=" << base << endl;
}

static const size_t ASSIGN4_CHUNK_COUNT = 524288 / sizeof(struct assign_vector4_event_s);
static slab_t<sizeof(assign_vector4_event_s),ASSIGN4_CHUNK_COUNT> assign4_heap;

inline void* assign_vector4_event_s::operator new(size_t size)
{
      assert(size == sizeof(assign_vector4_event_s));
      return assign4_heap.alloc_slab();
}

void assign_vector4_event_s::operator delete(void*dptr)
{
      assign4_heap.free_slab(dptr);
}

unsigned long count_assign4_pool(void) { return assign4_heap.pool; }

struct assign_vector8_event_s  : public event_s {
      vvp_net_ptr_t ptr;
      vvp_vector8_t val;
      void run_run(void);
      void single_step_display(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_vector8_event_s::run_run(void)
{
      count_assign_events += 1;
      vvp_send_vec8(ptr, val);
}

void assign_vector8_event_s::single_step_display(void)
{
      cerr << "assign_vector8_event: Propagate val=" << val << endl;
}

static const size_t ASSIGN8_CHUNK_COUNT = 8192 / sizeof(struct assign_vector8_event_s);
static slab_t<sizeof(assign_vector8_event_s),ASSIGN8_CHUNK_COUNT> assign8_heap;

inline void* assign_vector8_event_s::operator new(size_t size)
{
      assert(size == sizeof(assign_vector8_event_s));
      return assign8_heap.alloc_slab();
}

void assign_vector8_event_s::operator delete(void*dptr)
{
      assign8_heap.free_slab(dptr);
}

unsigned long count_assign8_pool() { return assign8_heap.pool; }

struct assign_real_event_s  : public event_s {
      vvp_net_ptr_t ptr;
      double val;
      void run_run(void);
      void single_step_display(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_real_event_s::run_run(void)
{
      count_assign_events += 1;
      vvp_send_real(ptr, val, 0);
}

void assign_real_event_s::single_step_display(void)
{
      cerr << "assign_real_event: Propagate val=" << val << endl;
}

static const size_t ASSIGNR_CHUNK_COUNT = 8192 / sizeof(struct assign_real_event_s);
static slab_t<sizeof(assign_real_event_s),ASSIGNR_CHUNK_COUNT> assignr_heap;

inline void* assign_real_event_s::operator new (size_t size)
{
      assert(size == sizeof(assign_real_event_s));
      return assignr_heap.alloc_slab();
}

void assign_real_event_s::operator delete(void*dptr)
{
      assignr_heap.free_slab(dptr);
}

unsigned long count_assign_real_pool(void) { return assignr_heap.pool; }

struct assign_array_word_s  : public event_s {
      vvp_array_t mem;
      unsigned adr;
      vvp_vector4_t val;
      unsigned off;
      void run_run(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_array_word_s::run_run(void)
{
      count_assign_events += 1;
      mem->set_word(adr, off, val);
}

static const size_t ARRAY_W_CHUNK_COUNT = 8192 / sizeof(struct assign_array_word_s);
static slab_t<sizeof(assign_array_word_s),ARRAY_W_CHUNK_COUNT> array_w_heap;

inline void* assign_array_word_s::operator new (size_t size)
{
      assert(size == sizeof(assign_array_word_s));
      return array_w_heap.alloc_slab();
}

void assign_array_word_s::operator delete(void*ptr)
{
      array_w_heap.free_slab(ptr);
}

unsigned long count_assign_aword_pool(void) { return array_w_heap.pool; }

struct force_vector4_event_s  : public event_s {
	/* The default constructor. */
      explicit force_vector4_event_s(const vvp_vector4_t&that): val(that) {
	    net = NULL;
	    base = 0;
	    vwid = 0;
      }
	/* Where to do the force. */
      vvp_net_t*net;
	/* Value to force. */
      vvp_vector4_t val;
	/* Offset of the part into the destination. */
      unsigned base;
	/* Width of the destination vector. */
      unsigned vwid;

      void run_run(void);
      void single_step_display(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void force_vector4_event_s::run_run(void)
{
      count_assign_events += 1;

      unsigned wid = val.size();
      if ((base + wid) > vwid)
	    wid = vwid - base;

	// Make a mask of which bits are to be forced, 0 for unforced
	// bits and 1 for forced bits.
      vvp_vector2_t mask (vvp_vector2_t::FILL0, vwid);
      for (unsigned idx = 0 ; idx < wid ; idx += 1)
	    mask.set_bit(base+idx, 1);

      vvp_vector4_t tmp (vwid, BIT4_Z);

	// vvp_net_t::force_vec4 propagates all the bits of the
	// forced vector value, regardless of the mask. This
	// ensures the unforced bits retain their current value.
      vvp_signal_value*sig = dynamic_cast<vvp_signal_value*>(net->fil);
      assert(sig);
      sig->vec4_value(tmp);

      tmp.set_vec(base, val);
      net->force_vec4(tmp, mask);
}

void force_vector4_event_s::single_step_display(void)
{
      cerr << "force_vector4_event: Force val=" << val
	   << ", vwid=" << vwid << ", base=" << base << endl;
}

static const size_t FORCE4_CHUNK_COUNT = 8192 / sizeof(struct force_vector4_event_s);
static slab_t<sizeof(force_vector4_event_s),FORCE4_CHUNK_COUNT> force4_heap;

inline void* force_vector4_event_s::operator new(size_t size)
{
      assert(size == sizeof(force_vector4_event_s));
      return force4_heap.alloc_slab();
}

void force_vector4_event_s::operator delete(void*dptr)
{
      force4_heap.free_slab(dptr);
}

unsigned long count_force4_pool(void) { return force4_heap.pool; }

/*
 * This class supports the propagation of vec4 outputs from a
 * vvp_net_t object.
 */
struct propagate_vector4_event_s : public event_s {
	/* The default constructor. */
      explicit propagate_vector4_event_s(const vvp_vector4_t&that) : val(that) {
	    net = NULL;
      }
	/* A constructor that makes the val directly. */
      propagate_vector4_event_s(const vvp_vector4_t&that, unsigned adr, unsigned wid)
      : val(that,adr,wid) {
	    net = NULL;
      }

	/* Propagate the output of this net. */
      vvp_net_t*net;
	/* value to propagate */
      vvp_vector4_t val;
	/* Action */
      void run_run(void);
      void single_step_display(void);
};

void propagate_vector4_event_s::run_run(void)
{
      net->send_vec4(val, 0);
}

void propagate_vector4_event_s::single_step_display(void)
{
      cerr << "propagate_vector4_event: Propagate val=" << val << endl;
}

/*
 * This class supports the propagation of real outputs from a
 * vvp_net_t object.
 */
struct propagate_real_event_s : public event_s {
	/* Propagate the output of this net. */
      vvp_net_t*net;
	/* value to propagate */
      double val;
	/* Action */
      void run_run(void);
      void single_step_display(void);
};

void propagate_real_event_s::run_run(void)
{
      net->send_real(val, 0);
}

void propagate_real_event_s::single_step_display(void)
{
      cerr << "propagate_real_event: Propagate val=" << val << endl;
}

struct assign_array_r_word_s  : public event_s {
      vvp_array_t mem;
      unsigned adr;
      double val;
      void run_run(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_array_r_word_s::run_run(void)
{
      count_assign_events += 1;
      mem->set_word(adr, val);
}
static const size_t ARRAY_R_W_CHUNK_COUNT = 8192 / sizeof(struct assign_array_r_word_s);
static slab_t<sizeof(assign_array_r_word_s),ARRAY_R_W_CHUNK_COUNT> array_r_w_heap;

inline void* assign_array_r_word_s::operator new(size_t size)
{
      assert(size == sizeof(assign_array_r_word_s));
      return array_r_w_heap.alloc_slab();
}

void assign_array_r_word_s::operator delete(void*ptr)
{
      array_r_w_heap.free_slab(ptr);
}

unsigned long count_assign_arword_pool(void) { return array_r_w_heap.pool; }

struct generic_event_s : public event_s {
      vvp_gen_event_t obj;
      bool delete_obj_when_done;
      void run_run(void);
      void single_step_display(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void generic_event_s::run_run(void)
{
      count_gen_events += 1;
      if (obj) {
	    obj->run_run();
	    if (delete_obj_when_done)
		  delete obj;
      }
}

void generic_event_s::single_step_display(void)
{
      obj->single_step_display();
}

static const size_t GENERIC_CHUNK_COUNT = 131072 / sizeof(struct generic_event_s);
static slab_t<sizeof(generic_event_s),GENERIC_CHUNK_COUNT> generic_event_heap;

inline void* generic_event_s::operator new(size_t size)
{
      assert(size == sizeof(generic_event_s));
      return generic_event_heap.alloc_slab();
}

void generic_event_s::operator delete(void*ptr)
{
      generic_event_heap.free_slab(ptr);
}

unsigned long count_gen_pool(void) { return generic_event_heap.pool; }

/*
** These event_time_s will be required a lot, at high frequency.
** Once allocated, we never free them, but stash them away for next time.
*/


static const size_t TIME_CHUNK_COUNT = 8192 / sizeof(struct event_time_s);
static slab_t<sizeof(event_time_s),TIME_CHUNK_COUNT> event_time_heap;

inline void* event_time_s::operator new (size_t size)
{
      assert(size == sizeof(struct event_time_s));
      void*ptr = event_time_heap.alloc_slab();
      return ptr;
}

inline void event_time_s::operator delete(void*ptr, size_t)
{
      event_time_heap.free_slab(ptr);
}

unsigned long count_time_pool(void) { return event_time_heap.pool; }

/*
 * This is the head of the list of pending events. This includes all
 * the events that have not been executed yet, and reaches into the
 * future.
 */
static struct event_time_s* sched_list = 0;

/*
 * This is a list of initialization events. The setup puts
 * initializations in this list so that they happen before the
 * simulation as a whole starts. This prevents time-0 triggers of
 * certain events.
 */
static struct event_s* schedule_init_list = 0;

/*
 * This is the head of the list of final events.
 */
static struct event_s* schedule_final_list = 0;

/*
 * This flag is true until a VPI task or function finishes the
 * simulation.
 */
static bool schedule_runnable = true;
static bool schedule_stopped_flag  = false;
static bool schedule_single_step_flag = false;
static bool no_signals_flag = false;

void schedule_finish(int)
{
      schedule_runnable = false;
}

void schedule_stop(int)
{
      schedule_stopped_flag = true;
}

void schedule_single_step(int)
{
      schedule_single_step_flag = true;
}

bool schedule_finished(void)
{
      return !schedule_runnable;
}

bool schedule_stopped(void)
{
      return schedule_stopped_flag;
}

extern "C" void vvp_no_signals(void)
{
      no_signals_flag = true;
}

/*
 * These are the signal handling infrastructure. The SIGINT signal
 * leads to an implicit $stop. The SIGHUP and SIGTERM signals lead
 * to an implicit $finish.
 */
extern bool stop_is_finish;

extern "C" void signals_handler(int signum)
{
#ifdef __MINGW32__
	// Windows implements the original UNIX semantics for signal,
	// so we have to re-establish the signal handler each time a
	// signal is caught.
      signal(signum, &signals_handler);
#endif
      if (signum != SIGINT)
	    stop_is_finish = true;
      schedule_stopped_flag = true;
}

static void signals_capture(void)
{
      if (no_signals_flag)
	   return;
#ifndef __MINGW32__
      signal(SIGHUP,  &signals_handler);
#endif
      signal(SIGINT,  &signals_handler);
      signal(SIGTERM, &signals_handler);
}

static void signals_revert(void)
{
      if (no_signals_flag)
	   return;
#ifndef __MINGW32__
      signal(SIGHUP,  SIG_DFL);
#endif
      signal(SIGINT,  SIG_DFL);
      signal(SIGTERM, SIG_DFL);
}

/*
 * This function puts an event on the end of the pre-simulation event queue.
 */
static void schedule_init_event(struct event_s*cur)
{
      if (schedule_init_list == 0) {
            cur->next = cur;
      } else {
            cur->next = schedule_init_list->next;
            schedule_init_list->next = cur;
      }
      schedule_init_list = cur;
}

/*
 * This function puts an event on the end of the post-simulation event queue.
 */
static void schedule_final_event(struct event_s*cur)
{
      if (schedule_final_list == 0) {
            cur->next = cur;
      } else {
            cur->next = schedule_final_list->next;
            schedule_final_list->next = cur;
      }
      schedule_final_list = cur;
}

/*
 * This function does all the hard work of putting an event into the
 * event queue. The event delay is taken from the event structure
 * itself, and the structure is placed in the right place in the
 * queue.
 */
typedef enum event_queue_e { SEQ_START, SEQ_ACTIVE, SEQ_INACTIVE, SEQ_NBASSIGN,
			     SEQ_RWSYNC, SEQ_ROSYNC, DEL_THREAD } event_queue_t;

static void schedule_event_(struct event_s*cur, vvp_time64_t delay,
			    event_queue_t select_queue)
{
      cur->next = cur;
      struct event_time_s*ctim = sched_list;

      if (sched_list == 0) {
	      /* Is the event_time list completely empty? Create the
		 first event_time object. */
	    ctim = new struct event_time_s;
	    ctim->delay = delay;
	    ctim->next  = 0;
	    sched_list = ctim;

      } else if (sched_list->delay > delay) {

	      /* Am I looking for an event before the first event_time?
		 If so, create a new event_time to go in front. */
	    struct event_time_s*tmp = new struct event_time_s;
	    tmp->delay = delay;
	    tmp->next = ctim;
	    ctim->delay -= delay;
	    ctim = tmp;
	    sched_list = ctim;

      } else {
	    struct event_time_s*prev = 0;

	    while (ctim->next && (ctim->delay < delay)) {
		  delay -= ctim->delay;
		  prev = ctim;
		  ctim = ctim->next;
	    }

	    if (ctim->delay > delay) {
		  struct event_time_s*tmp = new struct event_time_s;
		  tmp->delay = delay;
		  tmp->next  = prev->next;
		  prev->next = tmp;

		  tmp->next->delay -= delay;
		  ctim = tmp;

	    } else if (ctim->delay == delay) {

	    } else {
		  assert(ctim->next == 0);
		  struct event_time_s*tmp = new struct event_time_s;
		  tmp->delay = delay - ctim->delay;
		  tmp->next = 0;
		  ctim->next = tmp;

		  ctim = tmp;
	    }
      }

	/* By this point, ctim is the event_time structure that is to
	   receive the event at hand. Put the event in to the
	   appropriate list for the kind of assign we have at hand. */

      struct event_s** q = 0;

      switch (select_queue) {
	  case SEQ_START:
	    q = &ctim->start;
	    break;

	  case SEQ_ACTIVE:
	    q = &ctim->active;
	    break;

	  case SEQ_INACTIVE:
	    assert(delay == 0);
	    q = &ctim->inactive;
	    break;

	  case SEQ_NBASSIGN:
	    q = &ctim->nbassign;
	    break;

	  case SEQ_RWSYNC:
	    q = &ctim->rwsync;
	    break;

	  case SEQ_ROSYNC:
	    q = &ctim->rosync;
	    break;

	  case DEL_THREAD:
	    q = &ctim->del_thr;
	    break;
      }

      if (q) {
	    if (*q) {
		  /* Put the cur event on the end of the queue. */
		  cur->next = (*q)->next;
		  (*q)->next = cur;
	    }
	    *q = cur;
      }
}

static void schedule_event_push_(struct event_s*cur)
{
      if ((sched_list == 0) || (sched_list->delay > 0)) {
	    schedule_event_(cur, 0, SEQ_ACTIVE);
	    return;
      }

      struct event_time_s*ctim = sched_list;

      if (ctim->active == 0) {
	    cur->next = cur;
	    ctim->active = cur;
	    return;
      }

      cur->next = ctim->active->next;
      ctim->active->next = cur;
}

void schedule_vthread(vthread_t thr, vvp_time64_t delay, bool push_flag)
{
      struct vthread_event_s*cur = new vthread_event_s;

      cur->thr = thr;
      vthread_mark_scheduled(thr);

      if (push_flag && (delay == 0)) {
	      /* Special case: If the delay is 0, the push_flag means
		 I can push this event in front of everything. This is
		 used by the %fork statement, for example, to perform
		 task calls. */
	    schedule_event_push_(cur);

      } else {
	    schedule_event_(cur, delay, SEQ_ACTIVE);
      }
}

void schedule_t0_trigger(vvp_net_ptr_t ptr)
{
      vvp_vector4_t bit (1, BIT4_X);
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s(bit);
      cur->ptr = ptr;
      schedule_event_(cur, 0, SEQ_INACTIVE);
}

void schedule_inactive(vthread_t thr)
{
      struct vthread_event_s*cur = new vthread_event_s;

      cur->thr = thr;
      vthread_mark_scheduled(thr);
      schedule_event_(cur, 0, SEQ_INACTIVE);
}

void schedule_init_vthread(vthread_t thr)
{
      struct vthread_event_s*cur = new vthread_event_s;

      cur->thr = thr;
      vthread_mark_scheduled(thr);

      schedule_init_event(cur);
}

void schedule_final_vthread(vthread_t thr)
{
      struct vthread_event_s*cur = new vthread_event_s;

      cur->thr = thr;
      vthread_mark_final(thr);
      vthread_mark_scheduled(thr);

      schedule_final_event(cur);
}

void schedule_assign_vector(vvp_net_ptr_t ptr,
			    unsigned base, unsigned vwid,
			    const vvp_vector4_t&bit,
			    vvp_time64_t delay)
{
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s(bit);
      cur->ptr = ptr;
      cur->base = base;
      cur->vwid = vwid;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_force_vector(vvp_net_t*net,
			    unsigned base, unsigned vwid,
			    const vvp_vector4_t&bit,
			    vvp_time64_t delay)
{
      struct force_vector4_event_s*cur = new struct force_vector4_event_s(bit);
      cur->net = net;
      cur->base = base;
      cur->vwid = vwid;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_propagate_vector(vvp_net_t*net,
			       vvp_time64_t delay,
			       const vvp_vector4_t&src)
{
      struct propagate_vector4_event_s*cur
	    = new struct propagate_vector4_event_s(src);
      cur->net = net;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

// FIXME: This needs to create a non-blocking event, but only one per time slot.
//        Is schedule_event_ or execution actually filtering since the net is
//        already X because this is not triggering?
void schedule_propagate_event(vvp_net_t*net,
                              vvp_time64_t delay)
{
      vvp_vector4_t tmp (1, BIT4_X);
      struct propagate_vector4_event_s*cur
	    = new struct propagate_vector4_event_s(tmp);
      cur->net = net;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_assign_array_word(vvp_array_t mem,
				unsigned word_addr,
				unsigned off,
				const vvp_vector4_t&val,
				vvp_time64_t delay)
{
      struct assign_array_word_s*cur = new struct assign_array_word_s;
      cur->mem = mem;
      cur->adr = word_addr;
      cur->off = off;
      cur->val = val;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_assign_array_word(vvp_array_t mem,
				unsigned word_addr,
				double val,
				vvp_time64_t delay)
{
      struct assign_array_r_word_s*cur = new struct assign_array_r_word_s;
      cur->mem = mem;
      cur->adr = word_addr;
      cur->val = val;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_set_vector(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s(bit);
      cur->ptr = ptr;
      cur->base = 0;
      cur->vwid = 0;
      schedule_event_(cur, 0, SEQ_ACTIVE);
}

void schedule_set_vector(vvp_net_ptr_t ptr, const vvp_vector8_t&bit)
{
      struct assign_vector8_event_s*cur = new struct assign_vector8_event_s;
      cur->ptr = ptr;
      cur->val = bit;
      schedule_event_(cur, 0, SEQ_ACTIVE);
}

void schedule_set_vector(vvp_net_ptr_t ptr, double bit)
{
      struct assign_real_event_s*cur = new struct assign_real_event_s;
      cur->ptr = ptr;
      cur->val = bit;
      schedule_event_(cur, 0, SEQ_ACTIVE);
}

void schedule_init_vector(vvp_net_ptr_t ptr, const vvp_vector4_t&bit)
{
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s(bit);
      cur->ptr = ptr;
      cur->base = 0;
      cur->vwid = 0;
      schedule_init_event(cur);
}

void schedule_init_vector(vvp_net_ptr_t ptr, const vvp_vector8_t&bit)
{
      struct assign_vector8_event_s*cur = new struct assign_vector8_event_s;
      cur->ptr = ptr;
      cur->val = bit;
      schedule_init_event(cur);
}

void schedule_init_vector(vvp_net_ptr_t ptr, double bit)
{
      struct assign_real_event_s*cur = new struct assign_real_event_s;
      cur->ptr = ptr;
      cur->val = bit;
      schedule_init_event(cur);
}

void schedule_init_propagate(vvp_net_t*net, vvp_vector4_t bit)
{
      struct propagate_vector4_event_s*cur = new struct propagate_vector4_event_s(bit);
      cur->net = net;
      schedule_init_event(cur);
}

void schedule_init_propagate(vvp_net_t*net, double bit)
{
      struct propagate_real_event_s*cur = new struct propagate_real_event_s;
      cur->net = net;
      cur->val = bit;
      schedule_init_event(cur);
}

void schedule_del_thr(vthread_t thr)
{
      struct del_thr_event_s*cur = new del_thr_event_s;

      cur->thr = thr;

      schedule_event_(cur, 0, DEL_THREAD);
}

void schedule_generic(vvp_gen_event_t obj, vvp_time64_t delay,
		      bool sync_flag, bool ro_flag, bool delete_when_done)
{
      struct generic_event_s*cur = new generic_event_s;

      cur->obj = obj;
      cur->delete_obj_when_done = delete_when_done;
      schedule_event_(cur, delay,
		      sync_flag? (ro_flag?SEQ_ROSYNC:SEQ_RWSYNC) : SEQ_ACTIVE);

      if (sync_flag)
	    vthread_delay_delete();
}

static bool sim_started;

void schedule_functor(vvp_gen_event_t obj)
{
      struct generic_event_s*cur = new generic_event_s;

      cur->obj = obj;
      cur->delete_obj_when_done = false;
      if (!sim_started) {
            schedule_init_event(cur);
      } else {
            schedule_event_(cur, 0, SEQ_ACTIVE);
      }
}

void schedule_at_start_of_simtime(vvp_gen_event_t obj, vvp_time64_t delay)
{
      struct generic_event_s*cur = new generic_event_s;

      cur->obj = obj;
      cur->delete_obj_when_done = false;
      schedule_event_(cur, delay, SEQ_START);
}

/*
 * In the vvp runtime of Icarus Verilog, the SEQ_RWSYNC time step is
 * after all of the non-blocking assignments, so is effectively the
 * same as the ReadWriteSync time.
 */
void schedule_at_end_of_simtime(vvp_gen_event_t obj, vvp_time64_t delay)
{
      struct generic_event_s*cur = new generic_event_s;

      cur->obj = obj;
      cur->delete_obj_when_done = false;
      schedule_event_(cur, delay, SEQ_RWSYNC);
}

static vvp_time64_t schedule_time;
vvp_time64_t schedule_simtime(void)
{ return schedule_time; }

extern void vpiEndOfCompile();
extern void vpiStartOfSim();
extern void vpiPostsim();
extern void vpiNextSimTime(void);

static bool sim_at_rosync = false;
bool schedule_at_rosync(void)
{ return sim_at_rosync; }

/*
 * The scheduler uses this function to drain the rosync events of the
 * current time. The ctim object is still in the event queue, because
 * it is legal for a rosync callback to create other rosync
 * callbacks. It is *not* legal for them to create any other kinds of
 * events, and that is why the rosync is treated specially.
 *
 * Once all the rosync callbacks are done we can safely delete any
 * threads that finished during this time step.
 */
static void run_rosync(struct event_time_s*ctim)
{
      sim_at_rosync = true;
      while (ctim->rosync) {
	    struct event_s*cur = ctim->rosync->next;
	    if (cur->next == cur) {
		  ctim->rosync = 0;
	    } else {
		  ctim->rosync->next = cur->next;
	    }

	    cur->run_run();
	    delete cur;
      }
      sim_at_rosync = false;

      while (ctim->del_thr) {
	    struct event_s*cur = ctim->del_thr->next;
	    if (cur->next == cur) {
		  ctim->del_thr = 0;
	    } else {
		  ctim->del_thr->next = cur->next;
	    }

	    cur->run_run();
	    delete cur;
      }

      if (ctim->active || ctim->inactive || ctim->nbassign || ctim->rwsync) {
	    cerr << "SCHEDULER ERROR: read-only sync events "
		 << "created RW events!" << endl;
      }
}

void schedule_simulate(void)
{
      bool run_finals;
      sim_started = false;

      schedule_time = 0;

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ...execute EndOfCompile callbacks\n");
      }

      // Execute end of compile callbacks
      vpiEndOfCompile();

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ...propagate initialization events\n");
      }

	// Execute initialization events.
      while (schedule_init_list) {
	    struct event_s*cur = schedule_init_list->next;
	    if (cur->next == cur) {
		  schedule_init_list = 0;
	    } else {
		  schedule_init_list->next = cur->next;
	    }
	    cur->run_run();
	    delete cur;
      }

      sim_started = true;

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ...execute StartOfSim callbacks\n");
      }

      // Execute start of simulation callbacks
      vpiStartOfSim();

      signals_capture();

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ...run scheduler\n");
      }

      // If there were no compiletf, etc. errors then we are going to
      // process events and when done run the final blocks.
      run_finals = schedule_runnable;

      if (schedule_runnable) while (sched_list) {

	    if (schedule_stopped_flag) {
		  schedule_stopped_flag = false;
		  stop_handler(0);
		  // You can finish from the debugger without a time change.
		  if (!schedule_runnable) break;
		  continue;
	    }

	      /* ctim is the current time step. */
	    struct event_time_s* ctim = sched_list;

	      /* If the time is advancing, then first run the
		 postponed sync events. Run them all. */
	    if (ctim->delay > 0) {

		  if (!schedule_runnable) break;
		  schedule_time += ctim->delay;
		    /* When the design is being traced (we are emitting
		     * file/line information) also print any time changes. */
		  if (show_file_line) {
			cerr << "Advancing to simulation time: "
			     << schedule_time << endl;
		  }
		  ctim->delay = 0;

		  vpiNextSimTime();
		    // Process the cbAtStartOfSimTime callbacks.
		  while (ctim->start) {
			struct event_s*cur = ctim->start->next;
			if (cur->next == cur) {
			      ctim->start = 0;
			} else {
			      ctim->start->next = cur->next;
			}
			cur->run_run();
			delete (cur);
		  }
	    }


	      /* If there are no more active events, advance the event
		 queues. If there are not events at all, then release
		 the event_time object. */
	    if (ctim->active == 0) {
		  ctim->active = ctim->inactive;
		  ctim->inactive = 0;

		  if (ctim->active == 0) {
			ctim->active = ctim->nbassign;
			ctim->nbassign = 0;

			if (ctim->active == 0) {
			      ctim->active = ctim->rwsync;
			      ctim->rwsync = 0;

				/* If out of rw events, then run the rosync
				   events and delete this time step. This also
				   deletes threads as needed. */
			      if (ctim->active == 0) {
				    run_rosync(ctim);
				    sched_list = ctim->next;
				    delete ctim;
				    continue;
			      }
			}
		  }
	    }

	      /* Pull the first item off the list. If this is the last
		 cell in the list, then clear the list. Execute that
		 event type, and delete it. */
	    struct event_s*cur = ctim->active->next;
	    if (cur->next == cur) {
		  ctim->active = 0;
	    } else {
		  ctim->active->next = cur->next;
	    }

	    if (schedule_single_step_flag) {
		  cur->single_step_display();
		  schedule_stopped_flag = true;
		  schedule_single_step_flag = false;
	    }

	    cur->run_run();

	    delete (cur);
      }

	// Execute final events.
      schedule_runnable = run_finals;
      while (schedule_runnable && schedule_final_list) {
	    struct event_s*cur = schedule_final_list->next;
	    if (cur->next == cur) {
		  schedule_final_list = 0;
	    } else {
		  schedule_final_list->next = cur->next;
	    }
	    cur->run_run();
	    delete cur;
      }

      signals_revert();

      if (verbose_flag) {
	    vpi_mcd_printf(1, " ...execute Postsim callbacks\n");
      }

      // Execute post-simulation callbacks
      vpiPostsim();
#ifdef CHECK_WITH_VALGRIND
      schedule_delete();
#endif
}

#ifdef CHECK_WITH_VALGRIND
void schedule_delete(void)
{
      vthread_event_heap.delete_pool();
      assign4_heap.delete_pool();
      assign8_heap.delete_pool();
      assignr_heap.delete_pool();
      array_w_heap.delete_pool();
      array_r_w_heap.delete_pool();
      generic_event_heap.delete_pool();
      event_time_heap.delete_pool();
}
#endif
