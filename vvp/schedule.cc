/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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
# include  "functor.h"
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
 * stratified event queue. The event_time_s objects are one per time
 * step. Each time step in turn contains a list of event_s objects
 * that are the actual events.
 */
struct event_s {

      union {
	    vthread_t thr;
	    vvp_ipoint_t fun;
	    functor_t    funp;
            vvp_gen_event_t obj;
      };
      unsigned val  :2;
      unsigned type :2;

      struct event_s*next;

      void* operator new (size_t);
      void operator delete(void*obj, size_t s);
};
const unsigned TYPE_GEN    = 0;
const unsigned TYPE_THREAD = 1;
const unsigned TYPE_PROP   = 2;
const unsigned TYPE_ASSIGN = 3;

struct event_time_s {
      vvp_time64_t delay;

      struct event_s*active;
      struct event_s*nbassign;
      struct event_s*rosync;

      struct event_time_s*next;

      void* operator new (size_t);
      void operator delete(void*obj, size_t s);
};


/*
** These event_s will be required a lot, at high frequency.
** Once allocated, we never free them, but stash them away for next time.
*/

static struct event_s* event_free_list = 0;
static const unsigned CHUNK_COUNT = 8192 / sizeof(struct event_s);

inline void* event_s::operator new (size_t size)
{
      assert(size == sizeof(struct event_s));

      struct event_s* cur = event_free_list;
      if (!cur) {
	    cur = (struct event_s*)
		  malloc(CHUNK_COUNT * sizeof(struct event_s));
	    for (unsigned idx = 1 ;  idx < CHUNK_COUNT ;  idx += 1) {
		  cur[idx].next = event_free_list;
		  event_free_list = cur + idx;
	    }

	    count_event_pool += CHUNK_COUNT;

      } else {
	    event_free_list = cur->next;
      }

      return cur;
}

inline void event_s::operator delete(void*obj, size_t size)
{
      struct event_s*cur = reinterpret_cast<event_s*>(obj);
      cur->next = event_free_list;
      event_free_list = cur;
}

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
 * At the current time, events that are marked as synch events are put
 * into this list and held off until the time step is about to
 * advance. Then the events in this list are run and the clock is
 * allowed to advance.
 */
static struct event_s* synch_list = 0;


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
typedef enum event_queue_e { SEQ_ACTIVE, SEQ_NBASSIGN, SEQ_ROSYNC } event_queue_t;

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


/*
 */
static struct event_s* pull_sync_event(void)
{
      if (synch_list == 0)
	    return 0;

      struct event_s*cur = synch_list->next;
      if (cur->next == cur) {
	    synch_list = 0;
      } else {
	    synch_list->next = cur->next;
      }

      return cur;
}

void schedule_vthread(vthread_t thr, vvp_time64_t delay, bool push_flag)
{
      struct event_s*cur = new event_s;

      cur->thr = thr;
      cur->type = TYPE_THREAD;
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

void functor_s::schedule(vvp_time64_t delay, bool nba_flag)
{
      struct event_s*cur = new event_s;

      cur->funp = this;
      cur->type = TYPE_PROP;

      schedule_event_(cur, delay, nba_flag? SEQ_NBASSIGN:SEQ_ACTIVE);
}

void schedule_assign(vvp_ipoint_t fun, unsigned char val, vvp_time64_t delay)
{
      struct event_s*cur = new event_s;

      cur->fun = fun;
      cur->val = val;
      cur->type= TYPE_ASSIGN;

      schedule_event_(cur, delay, SEQ_NBASSIGN);
}

void schedule_generic(vvp_gen_event_t obj, unsigned char val,
		      vvp_time64_t delay, bool sync_flag)
{
      struct event_s*cur = new event_s;

      cur->obj = obj;
      cur->val = val;
      cur->type= TYPE_GEN;

      schedule_event_(cur, delay, sync_flag? SEQ_ROSYNC : SEQ_ACTIVE);
}

static vvp_time64_t schedule_time;
vvp_time64_t schedule_simtime(void)
{ return schedule_time; }

extern void vpiPresim();
extern void vpiPostsim();
extern void vpiNextSimTime(void);

void schedule_simulate(void)
{
      schedule_time = 0;

      // Execute pre-simulation callbacks
      vpiPresim();

      signals_capture();

      if (schedule_runnable) while (sched_list) {

	    if (schedule_stopped_flag) {
		  schedule_stopped_flag = false;
		  stop_handler(0);
		  if (!schedule_runnable) break;
		  continue;
	    }

	      /* ctim is the current time step. */
	    struct event_time_s* ctim = sched_list;

	      /* If the time is advancing, then first run the
		 postponed sync events. Run them all. */
	    if (ctim->delay > 0) {

		  if (!schedule_runnable) break;
		  struct event_s*sync_cur;
		  while ( (sync_cur = pull_sync_event()) ) {
			assert(sync_cur->type == TYPE_GEN);
			if (sync_cur->obj && sync_cur->obj->run) {
			      sync_cur->obj->run(sync_cur->obj, sync_cur->val);
			}
			delete sync_cur;
		  }


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
			sched_list = ctim->next;
			synch_list = ctim->rosync;
			delete ctim;
			continue;
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

	    switch (cur->type) {
		case TYPE_THREAD:
		  count_thread_events += 1;
		  vthread_run(cur->thr);
		  break;

		case TYPE_PROP:
		    //printf("Propagate %p\n", cur->fun);
		  count_prop_events += 1;
		  cur->funp->propagate(false);
		  break;

		case TYPE_ASSIGN:
		  count_assign_events += 1;
		  { static const unsigned val_table[4] = {St0, St1, StX, HiZ};
		    functor_set(cur->fun,
				cur->val,
				val_table[cur->val],
				false);
		  }
		  break;

		case TYPE_GEN:
		  count_gen_events += 1;
		  if (cur->obj && cur->obj->run)
			cur->obj->run(cur->obj, cur->val);

		  break;

	    }

	    delete (cur);
      }

	/* Clean up lingering ReadOnlySync events. It is safe to do
	   that out here because ReadOnlySync events are not allowed
	   to create new events. */
      for (struct event_s*sync_cur = pull_sync_event()
		 ; sync_cur ;  sync_cur = pull_sync_event()) {

	    assert(sync_cur->type == TYPE_GEN);

	    if (sync_cur->obj && sync_cur->obj->run)
		  sync_cur->obj->run(sync_cur->obj, sync_cur->val);

	    delete (sync_cur);
      }


      signals_revert();

      // Execute post-simulation callbacks
      vpiPostsim();
}
