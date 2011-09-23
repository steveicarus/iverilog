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

# include  "schedule.h"
# include  "vthread.h"
# include  "slab.h"
# include  <new>
# include  <typeinfo>
# include  <csignal>
# include  <cstdlib>
# include  <cassert>

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

	// Fallback new/delete
      static void*operator new (size_t size) { return ::new char[size]; }
      static void operator delete(void*ptr)  { ::delete[]( (char*)ptr ); }
};

struct event_time_s {
      event_time_s() {
	    count_time_events += 1;
	    start = 0;
	    active = 0;
	    nbassign = 0;
	    rwsync = 0;
	    rosync = 0;
	    del_thr = 0;
      }
      vvp_time64_t delay;

      struct event_s*start;
      struct event_s*active;
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

/*
 * Derived event types
 */
struct vthread_event_s : public event_s {
      vthread_t thr;
      void run_run(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void vthread_event_s::run_run(void)
{
      count_thread_events += 1;
      vthread_run(thr);
}

static const size_t VTHR_CHUNK_COUNT = 8192 / sizeof(struct vthread_event_s);
static slab_t<sizeof(vthread_event_s),VTHR_CHUNK_COUNT> vthread_event_heap;

inline void* vthread_event_s::operator new(size_t size)
{
      assert(size == sizeof(vthread_event_s));
      return vthread_event_heap.alloc_slab();
}

void vthread_event_s::operator delete(void*ptr)
{
      vthread_event_heap.free_slab(ptr);
}

struct del_thr_event_s : public event_s {
      vthread_t thr;
      void run_run(void);
};

void del_thr_event_s::run_run(void)
{
      vthread_delete(thr);
}

struct assign_vector4_event_s  : public event_s {
	/* The default constructor. */
      assign_vector4_event_s(const vvp_vector4_t&that) : val(that) { }
	/* A constructor that makes the val directly. */
      assign_vector4_event_s(const vvp_vector4_t&that, unsigned adr, unsigned wid)
      : val(that,adr,wid) { }

	/* Where to do the assign. */
      vvp_net_ptr_t ptr;
	/* Value to assign. */
      vvp_vector4_t val;
	/* Offset of the part into the destination. */
      unsigned base;
	/* Width of the destination vector. */
      unsigned vwid;
      void run_run(void);

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_vector4_event_s::run_run(void)
{
      count_assign_events += 1;
      if (vwid > 0)
	    vvp_send_vec4_pv(ptr, val, base, val.size(), vwid, 0);
      else
	    vvp_send_vec4(ptr, val, 0);
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

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_vector8_event_s::run_run(void)
{
      count_assign_events += 1;
      vvp_send_vec8(ptr, val);
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

      static void* operator new(size_t);
      static void operator delete(void*);
};

void assign_real_event_s::run_run(void)
{
      count_assign_events += 1;
      vvp_send_real(ptr, val, 0);
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
      array_set_word(mem, adr, off, val);
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

struct generic_event_s : public event_s {
      vvp_gen_event_t obj;
      bool delete_obj_when_done;
      void run_run(void);

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

inline void event_time_s::operator delete(void*ptr, size_t size)
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
 * This flag is true until a VPI task or function finishes the
 * simulation.
 */
static bool schedule_runnable = true;
static bool schedule_stopped_flag  = false;

void schedule_finish(int)
{
      schedule_runnable = false;
}

void schedule_stop(int)
{
      schedule_stopped_flag = true;
}

bool schedule_finished(void)
{
      return !schedule_runnable;
}

bool schedule_stopped(void)
{
      return schedule_stopped_flag;
}

/*
 * These are the signal handling infrastructure. The SIGINT signal
 * leads to an implicit $stop.
 */
extern "C" void signals_handler(int)
{
      schedule_stopped_flag = true;
}

static void signals_capture(void)
{
      signal(SIGINT, &signals_handler);
}

static void signals_revert(void)
{
      signal(SIGINT, SIG_DFL);
}


/*
 * This function does all the hard work of putting an event into the
 * event queue. The event delay is taken from the event structure
 * itself, and the structure is placed in the right place in the
 * queue.
 */
typedef enum event_queue_e { SEQ_START, SEQ_ACTIVE, SEQ_NBASSIGN,
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

      switch (select_queue) {

	  case SEQ_START:
	    if (ctim->start == 0) {
		  ctim->start = cur;
	    } else {
		  cur->next = ctim->active->next;
		  ctim->active->next = cur;
		  ctim->active = cur;
	    }
	    break;

	  case SEQ_ACTIVE:
	    if (ctim->active == 0) {
		  ctim->active = cur;

	    } else {
		    /* Put the cur event on the end of the active list. */
		  cur->next = ctim->active->next;
		  ctim->active->next = cur;
		  ctim->active = cur;
	    }
	    break;

	  case SEQ_NBASSIGN:
	    if (ctim->nbassign == 0) {
		  ctim->nbassign = cur;

	    } else {
		    /* Put the cur event on the end of the active list. */
		  cur->next = ctim->nbassign->next;
		  ctim->nbassign->next = cur;
		  ctim->nbassign = cur;
	    }
	    break;

	  case SEQ_RWSYNC:
	    if (ctim->rwsync == 0) {
		  ctim->rwsync = cur;

	    } else {
		    /* Put the cur event on the end of the active list. */
		  cur->next = ctim->rwsync->next;
		  ctim->rwsync->next = cur;
		  ctim->rwsync = cur;
	    }
	    break;

	  case SEQ_ROSYNC:
	    if (ctim->rosync == 0) {
		  ctim->rosync = cur;

	    } else {
		    /* Put the cur event on the end of the active list. */
		  cur->next = ctim->rosync->next;
		  ctim->rosync->next = cur;
		  ctim->rosync = cur;
	    }
	    break;

	  case DEL_THREAD:
	    if (ctim->del_thr == 0) {
		  ctim->del_thr = cur;

	    } else {
		    /* Put the cur event on the end of the active list. */
		  cur->next = ctim->del_thr->next;
		  ctim->del_thr->next = cur;
		  ctim->del_thr = cur;
	    }
	    break;
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

void schedule_assign_plucked_vector(vvp_net_ptr_t ptr,
				    vvp_time64_t delay,
				    const vvp_vector4_t&src,
				    unsigned adr, unsigned wid)
{
      struct assign_vector4_event_s*cur
	    = new struct assign_vector4_event_s(src,adr,wid);
      cur->ptr = ptr;
      cur->vwid = 0;
      cur->base = 0;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_assign_array_word(vvp_array_t mem,
				unsigned word_addr,
				unsigned off,
				vvp_vector4_t val,
				vvp_time64_t delay)
{
      struct assign_array_word_s*cur = new struct assign_array_word_s;
      cur->mem = mem;
      cur->adr = word_addr;
      cur->off = off;
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

void schedule_set_vector(vvp_net_ptr_t ptr, vvp_vector8_t bit)
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

void schedule_init_vector(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s(bit);
      cur->ptr = ptr;
      cur->base = 0;
      cur->vwid = 0;
      cur->next = schedule_init_list;
      schedule_init_list = cur;
}

void schedule_init_vector(vvp_net_ptr_t ptr, double bit)
{
      struct assign_real_event_s*cur = new struct assign_real_event_s;
      cur->ptr = ptr;
      cur->val = bit;
      cur->next = schedule_init_list;
      schedule_init_list = cur;
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

void schedule_at_start_of_simtime(vvp_gen_event_t obj, vvp_time64_t delay)
{
      struct generic_event_s*cur = new generic_event_s;

      cur->obj = obj;
      cur->delete_obj_when_done = false;
      schedule_event_(cur, delay, SEQ_START);
}

static vvp_time64_t schedule_time;
vvp_time64_t schedule_simtime(void)
{ return schedule_time; }

extern void vpiEndOfCompile();
extern void vpiStartOfSim();
extern void vpiPostsim();
extern void vpiNextSimTime(void);

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

      if (ctim->active || ctim->nbassign || ctim->rwsync) {
	    fprintf(stderr, "SCHEDULER ERROR: read-only sync events "
		    "created RW events!\n");
      }
}

void schedule_simulate(void)
{
      schedule_time = 0;

      // Execute end of compile callbacks
      vpiEndOfCompile();

	// Execute initialization events.
      while (schedule_init_list) {
	    struct event_s*cur = schedule_init_list;
	    schedule_init_list = cur->next;

	    cur->run_run();
	    delete cur;
      }

      // Execute start of simulation callbacks
      vpiStartOfSim();

      signals_capture();

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

	      /* Pull the first item off the list. If this is the last
		 cell in the list, then clear the list. Execute that
		 event type, and delete it. */
	    struct event_s*cur = ctim->active->next;
	    if (cur->next == cur) {
		  ctim->active = 0;
	    } else {
		  ctim->active->next = cur->next;
	    }

	    cur->run_run();

	    delete (cur);
      }


      signals_revert();

      // Execute post-simulation callbacks
      vpiPostsim();
}
