/*
 * Copyright (c) 2001-2007 Stephen Williams (steve@icarus.com)
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
#ident "$Id: schedule.cc,v 1.45 2007/01/16 05:44:16 steve Exp $"
#endif

# include  "schedule.h"
# include  "memory.h"
# include  "vthread.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <signal.h>
# include  <stdlib.h>
# include  <assert.h>

# include  <stdio.h>

unsigned long count_assign_events = 0;
unsigned long count_gen_events = 0;
unsigned long count_prop_events = 0;
unsigned long count_thread_events = 0;
unsigned long count_event_pool = 0;
unsigned long count_time_pool = 0;


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
};

struct event_time_s {
      vvp_time64_t delay;

      struct event_s*active;
      struct event_s*nbassign;
      struct event_s*rwsync;
      struct event_s*rosync;

      struct event_time_s*next;

      void* operator new (size_t);
      void operator delete(void*obj, size_t s);
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
};

void vthread_event_s::run_run(void)
{
      count_thread_events += 1;
      vthread_run(thr);
}

struct assign_vector4_event_s  : public event_s {
	/* Where to do the assign. */
      vvp_net_ptr_t ptr;
	/* Value to assign. */
      vvp_vector4_t val;
	/* Offset of the part into the destination. */
      unsigned base;
	/* Width of the destination vector. */
      unsigned vwid;
      void run_run(void);
};

void assign_vector4_event_s::run_run(void)
{
      count_assign_events += 1;
      if (vwid > 0)
	    vvp_send_vec4_pv(ptr, val, base, val.size(), vwid);
      else
	    vvp_send_vec4(ptr, val);
}

struct assign_vector8_event_s  : public event_s {
      vvp_net_ptr_t ptr;
      vvp_vector8_t val;
      void run_run(void);
};

void assign_vector8_event_s::run_run(void)
{
      count_assign_events += 1;
      vvp_send_vec8(ptr, val);
}

struct assign_real_event_s  : public event_s {
      vvp_net_ptr_t ptr;
      double val;
      void run_run(void);
};

void assign_real_event_s::run_run(void)
{
      count_assign_events += 1;
      vvp_send_real(ptr, val);
}

struct assign_memory_word_s  : public event_s {
      vvp_memory_t mem;
      unsigned adr;
      vvp_vector4_t val;
      unsigned off;
      void run_run(void);
};

void assign_memory_word_s::run_run(void)
{
      count_assign_events += 1;
      memory_set_word(mem, adr, off, val);
}

struct assign_array_word_s  : public event_s {
      vvp_array_t mem;
      unsigned adr;
      vvp_vector4_t val;
      unsigned off;
      void run_run(void);
};

void assign_array_word_s::run_run(void)
{
      count_assign_events += 1;
      array_set_word(mem, adr, off, val);
}

struct generic_event_s : public event_s {
      vvp_gen_event_t obj;
      unsigned char val;
      void run_run(void);
};

void generic_event_s::run_run(void)
{
      count_gen_events += 1;
      if (obj)
	    obj->run_run();
}

/*
** These event_time_s will be required a lot, at high frequency.
** Once allocated, we never free them, but stash them away for next time.
*/


static struct event_time_s* time_free_list = 0;
static const unsigned TIME_CHUNK_COUNT = 8192 / sizeof(struct event_time_s);

inline void* event_time_s::operator new (size_t size)
{
      assert(size == sizeof(struct event_time_s));

      struct event_time_s* cur = time_free_list;
      if (!cur) {
	    cur = (struct event_time_s*)
		  malloc(TIME_CHUNK_COUNT * sizeof(struct event_time_s));
	    for (unsigned idx = 1 ;  idx < TIME_CHUNK_COUNT ;  idx += 1) {
		  cur[idx].next = time_free_list;
		  time_free_list = cur + idx;
	    }

	    count_time_pool += TIME_CHUNK_COUNT;

      } else {
	    time_free_list = cur->next;
      }

      return cur;
}

inline void event_time_s::operator delete(void*obj, size_t size)
{
      struct event_time_s*cur = reinterpret_cast<event_time_s*>(obj);
      cur->next = time_free_list;
      time_free_list = cur;
}

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
static void signals_handler(int)
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
typedef enum event_queue_e { SEQ_ACTIVE, SEQ_NBASSIGN, SEQ_RWSYNC, SEQ_ROSYNC } event_queue_t;

static void schedule_event_(struct event_s*cur, vvp_time64_t delay,
			    event_queue_t select_queue)
{
      cur->next = cur;

      struct event_time_s*ctim = sched_list;

      if (sched_list == 0) {
	      /* Is the event_time list completely empty? Create the
		 first event_time object. */
	    ctim = new struct event_time_s;
	    ctim->active = 0;
	    ctim->nbassign = 0;
	    ctim->rwsync = 0;
	    ctim->rosync = 0;
	    ctim->delay = delay;
	    ctim->next  = 0;
	    sched_list = ctim;

      } else if (sched_list->delay > delay) {

	      /* Am I looking for an event before the first event_time?
		 If so, create a new event_time to go in front. */
	    struct event_time_s*tmp = new struct event_time_s;
	    tmp->active = 0;
	    tmp->nbassign = 0;
	    tmp->rwsync = 0;
	    tmp->rosync = 0;
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
		  tmp->active = 0;
		  tmp->nbassign = 0;
		  tmp->rwsync = 0;
		  tmp->rosync = 0;
		  tmp->delay = delay;
		  tmp->next  = prev->next;
		  prev->next = tmp;

		  tmp->next->delay -= delay;
		  ctim = tmp;

	    } else if (ctim->delay == delay) {

	    } else {
		  assert(ctim->next == 0);
		  struct event_time_s*tmp = new struct event_time_s;
		  tmp->active = 0;
		  tmp->nbassign = 0;
		  tmp->rwsync = 0;
		  tmp->rosync = 0;
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
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s;
      cur->ptr = ptr;
      cur->base = base;
      cur->vwid = vwid;
      cur->val = bit;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_assign_vector(vvp_net_ptr_t ptr,
			    const vvp_vector4_t&bit,
			    vvp_time64_t delay)
{
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s;
      cur->ptr = ptr;
      cur->val = bit;
      cur->vwid = 0;
      cur->base = 0;
      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_assign_memory_word(vvp_memory_t mem,
				 unsigned word_addr,
				 unsigned off,
				 vvp_vector4_t val,
				 vvp_time64_t delay)
{
      struct assign_memory_word_s*cur = new struct assign_memory_word_s;
      cur->mem = mem;
      cur->adr = word_addr;
      cur->off = off;
      cur->val = val;
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

void schedule_set_vector(vvp_net_ptr_t ptr, vvp_vector4_t bit)
{
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s;
      cur->ptr = ptr;
      cur->val = bit;
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
      struct assign_vector4_event_s*cur = new struct assign_vector4_event_s;
      cur->ptr = ptr;
      cur->val = bit;
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

void schedule_generic(vvp_gen_event_t obj, vvp_time64_t delay,
		      bool sync_flag, bool ro_flag)
{
      struct generic_event_s*cur = new generic_event_s;

      cur->obj = obj;
      cur->val = 0;

      schedule_event_(cur, delay,
		      sync_flag? (ro_flag?SEQ_ROSYNC:SEQ_RWSYNC) : SEQ_ACTIVE);
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

      while (sched_list) {

	    if (schedule_stopped_flag) {
		  schedule_stopped_flag = false;
		  stop_handler(0);
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
			     events and delete this timestep. */
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

