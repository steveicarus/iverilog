/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: schedule.cc,v 1.23 2003/02/21 03:40:35 steve Exp $"
#endif

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

/*
 * The event queue is arranged as a skip list, with the simulation
 * time the key to the list. The simulation time is stored in each
 * event as the delta time from the previous event so that there is no
 * limit to the time values that are supported.
 *
 * The list is started by the ``list'' variable below. This points to
 * the very next event to be executed.  Each event, in turn, points to
 * the next item in the event queue with the ->next member.
 *
 * The ->last member points to the last event in the current
 * time. That is, all the events to and including the ->last event are
 * zero delay from the current event.
 */
struct event_s {
      unsigned delay;

      union {
	    vthread_t thr;
	    vvp_ipoint_t fun;
	    functor_t    funp;
            vvp_gen_event_t obj;
      };
      unsigned val  :2;
      unsigned type :2;
      // unsigned char str;

      struct event_s*next;
      struct event_s*last;

      void* operator new (size_t);
      void operator delete(void*obj, size_t s);
};
const unsigned TYPE_GEN    = 0;
const unsigned TYPE_THREAD = 1;
const unsigned TYPE_PROP   = 2;
const unsigned TYPE_ASSIGN = 3;

/*
** These event_s will be required a lot, at high frequency.
** Once allocated, we never free them, but stash them away for next time. 
*/

static struct event_s* free_list = 0;
static const unsigned CHUNK_COUNT = 8192 / sizeof(struct event_s);

inline void* event_s::operator new (size_t size)
{
      assert(size == sizeof(struct event_s));

      struct event_s* cur = free_list;
      if (!cur) {
	    cur = (struct event_s*)
		  malloc(CHUNK_COUNT * sizeof(struct event_s));
	    for (unsigned idx = 1 ;  idx < CHUNK_COUNT ;  idx += 1) {
		  cur[idx].next = free_list;
		  free_list = cur + idx;
	    }

	    count_event_pool += CHUNK_COUNT;

      } else {
	    free_list = cur->next;
      }

      return cur;
}

inline void event_s::operator delete(void*obj, size_t size)
{
      struct event_s*cur = reinterpret_cast<event_s*>(obj);
      cur->next = free_list;
      free_list = cur;
}

/*
 * This is the head of the list of pending events. This includes all
 * the events that have not been executed yet, and reaches into the
 * future.
 */
static struct event_s* sched_list = 0;

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
static bool schedule_stopped  = false;

void schedule_finish(int)
{
      schedule_runnable = false;
}

void schedule_stop(int)
{
      schedule_stopped = true;
}

bool schedule_finished(void)
{
      return !schedule_runnable;
}

/*
 * These are the signal handling infrastructure. The SIGINT signal
 * leads to an implicit $stop.
 */
static void signals_handler(int)
{
      schedule_stopped = true;
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
static void schedule_event_(struct event_s*cur)
{
      cur->last = cur;

	/* If the list is completely empty, then start the list with
	   this the only event. */
      if (sched_list == 0) {
	    sched_list = cur;
	    cur->next = 0;
	    return;
      }

      struct event_s*idx = sched_list;
      if (cur->delay < idx->delay) {
	      /* If this new event is earlier then even the first
		 event, then insert it in front. Adjust the delay of
		 the next event, and set the start to me. */
	    idx->delay -= cur->delay;
	    cur->next = idx;
	    sched_list = cur;

      } else {

	      /* Look for the first event after the cur
		 event. Decrease the cur->delay as I go, and use the
		 skip member to accelerate the search. When I'm done,
		 prev will point to the event immediately before where
		 this event goes. */
	    struct event_s*prev = idx;
	    while (cur->delay > idx->delay) {
		  cur->delay -= idx->delay;
		  prev = idx->last;
		  if (prev->next == 0) {
			cur->next = 0;
			prev->next = cur;
			return;
		  }
		  idx = prev->next;
	    }

	    if (cur->delay < idx->delay) {
		  idx->delay -= cur->delay;
		    //cur->last = cur;
		  cur->next = idx;
		  prev->next = cur;

	    } else {
		  assert(cur->delay == idx->delay);
		  cur->delay = 0;
		    //cur->last = cur;
		  cur->next = idx->last->next;
		  idx->last->next = cur;
		  idx->last = cur;
	    }
      }
}
static void schedule_event_push_(struct event_s*cur)
{
      assert(cur->delay == 0);
      cur->last = cur;

      if (sched_list == 0) {
	    sched_list = cur;
	    cur->next = 0;
	    return;
      }

      if (sched_list->delay == 0)
	    cur->last = sched_list->last;
      cur->next = sched_list;
      sched_list = cur;
}


/*
 * The synch_list is managed as a doubly-linked circular list. There is
 * no need for the skip capabilities, so use the "last" member as a
 * prev pointer. This function appends the event to the synch_list.
 */
static void postpone_sync_event(struct event_s*cur)
{
      if (synch_list == 0) {
	    synch_list = cur;
	    cur->next = cur;
	    cur->last = cur;
	    return;
      }

      cur->next = synch_list;
      cur->last = synch_list->last;
      cur->next->last = cur;
      cur->last->next = cur;
}

static struct event_s* pull_sync_event(void)
{
      if (synch_list == 0)
	    return 0;

      struct event_s*cur = synch_list;
      synch_list = cur->next;
      if (cur == synch_list) {
	    synch_list = 0;
      } else {
	    cur->next->last = cur->last;
	    cur->last->next = cur->next;
      }

      return cur;
}

void schedule_vthread(vthread_t thr, unsigned delay, bool push_flag)
{
      struct event_s*cur = new event_s;

      cur->delay = delay;
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
	    schedule_event_(cur);
      }
}

void functor_s::schedule(unsigned delay)
{
      struct event_s*cur = new event_s;

      cur->delay = delay;
      cur->funp = this;
      cur->type = TYPE_PROP;
      // cur->str = get_ostr();
      // cur->val = get_oval();

      schedule_event_(cur);
}

void schedule_assign(vvp_ipoint_t fun, unsigned char val, unsigned delay)
{
      struct event_s*cur = new event_s;

      cur->delay = delay;
      cur->fun = fun;
      cur->val = val;
      cur->type= TYPE_ASSIGN;

      schedule_event_(cur);
}

void schedule_generic(vvp_gen_event_t obj, unsigned char val, unsigned delay)
{
      struct event_s*cur = new event_s;

      cur->delay = delay;
      cur->obj = obj;
      cur->val = val;
      cur->type= TYPE_GEN;

      schedule_event_(cur);
}

static vvp_time64_t schedule_time;
vvp_time64_t schedule_simtime(void)
{ return schedule_time; }

extern void vpiPresim();
extern void vpiPostsim();

void schedule_simulate(void)
{
      schedule_time = 0;

      // Execute pre-simulation callbacks
      vpiPresim();

      signals_capture();

      while (schedule_runnable && sched_list) {

	    if (schedule_stopped) {
		  schedule_stopped = false;
		  stop_handler(0);
		  continue;
	    }

	      /* Pull the first item off the list. Fixup the last
		 pointer in the next cell, if necessary. */
	    struct event_s*cur = sched_list;
	    sched_list = cur->next;
	    if (cur->last != cur) {
		  assert(sched_list);
		  assert(sched_list->delay == 0);
		  sched_list->last = cur->last;

	    }

	      /* If the time is advancing, then first run the
		 postponed sync events. Run them all. */
	    if (cur->delay > 0) {
		  struct event_s*sync_cur;
		  while ( (sync_cur = pull_sync_event()) ) {
			assert(sync_cur->type == TYPE_GEN);
			if (sync_cur->obj && sync_cur->obj->run) {
			      assert(sync_cur->obj->sync_flag);
			      sync_cur->obj->run(sync_cur->obj, sync_cur->val);
			}
			delete sync_cur;
		  }


		  schedule_time += cur->delay;
		    //printf("TIME: %u\n", schedule_time);
	    }

	    switch (cur->type) {
		case TYPE_THREAD:
		  count_thread_events += 1;
		  vthread_run(cur->thr);
		  delete cur;
		  break;

		case TYPE_PROP:
		    //printf("Propagate %p\n", cur->fun);
		  count_prop_events += 1;
		  cur->funp->propagate(false);
		  delete(cur);
		  break;

		case TYPE_ASSIGN:
		  count_assign_events += 1;
		  switch (cur->val) {
		      case 0:
			functor_set(cur->fun, cur->val, St0, false);
			break;
		      case 1:
			functor_set(cur->fun, cur->val, St1, false);
			break;
		      case 2:
			functor_set(cur->fun, cur->val, StX, false);
			break;
		      case 3:
			functor_set(cur->fun, cur->val, HiZ, false);
			break;
		  }
		  delete(cur);
		  break;

		case TYPE_GEN:
		  count_gen_events += 1;
		  if (cur->obj && cur->obj->run) {
			if (cur->obj->sync_flag == false) {
			      cur->obj->run(cur->obj, cur->val);
			      delete (cur);

			} else {
			      postpone_sync_event(cur);
			}
		  }
		  break;

	    }
      }

	/* Clean up lingering ReadOnlySync events. It is safe to do
	   that out here because ReadOnlySync events are not allowed
	   to create new events. */
      for (struct event_s*sync_cur = pull_sync_event()
		 ; sync_cur ;  sync_cur = pull_sync_event()) {

	    assert(sync_cur->type == TYPE_GEN);

	    if (sync_cur->obj && sync_cur->obj->run) {
		  assert(sync_cur->obj->sync_flag);
		  sync_cur->obj->run(sync_cur->obj, sync_cur->val);
	    }

	    delete (sync_cur);
      }


      signals_revert();

      // Execute post-simulation callbacks
      vpiPostsim();
}

/*
 * $Log: schedule.cc,v $
 * Revision 1.23  2003/02/21 03:40:35  steve
 *  Add vpiStop and interactive mode.
 *
 * Revision 1.22  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.21  2003/01/06 23:57:26  steve
 *  Schedule wait lists of threads as a single event,
 *  to save on events. Also, improve efficiency of
 *  event_s allocation. Add some event statistics to
 *  get an idea where performance is really going.
 *
 * Revision 1.20  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.19  2002/07/31 03:22:44  steve
 *  Account for the tail readonly callbacks.
 *
 * Revision 1.18  2002/05/12 23:44:41  steve
 *  task calls and forks push the thread event in the queue.
 *
 * Revision 1.17  2002/05/04 03:03:17  steve
 *  Add simulator event callbacks.
 *
 * Revision 1.16  2002/04/20 04:33:23  steve
 *  Support specified times in cbReadOnlySync, and
 *  add support for cbReadWriteSync.
 *  Keep simulation time in a 64bit number.
 */

