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
#if !defined(WINNT)
#ident "$Id: schedule.cc,v 1.12 2001/09/15 18:27:05 steve Exp $"
#endif

# include  "schedule.h"
# include  "functor.h"
# include  "memory.h"
# include  "vthread.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <assert.h>

# include  <stdio.h>
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
            vvp_gen_event_t obj;
      };
      unsigned val  :2;
      unsigned type :2;

      struct event_s*next;
      struct event_s*last;
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

inline static struct event_s* e_alloc()
{
  struct event_s* cur = free_list;
  if (!cur)
    {
      cur = (struct event_s*) malloc(sizeof(struct event_s));
      // cur = (struct event_s*) calloc(1, sizeof(struct event_s));
    }
  else
    {
      free_list = cur->next;
      // memset(cur, 0, sizeof(struct event_s));
    }
  return cur;
}

inline static void e_free(struct event_s* cur)
{
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

void schedule_finish(int)
{
      schedule_runnable = false;
}

bool schedule_finished(void)
{
      return !schedule_runnable;
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
		 skip member to accellerate the search. When I'm done,
		 prev will point to the even immediately before where
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

void schedule_vthread(vthread_t thr, unsigned delay)
{
      struct event_s*cur = e_alloc();

      cur->delay = delay;
      cur->thr = thr;
      cur->type = TYPE_THREAD;
      vthread_mark_scheduled(thr);

      schedule_event_(cur);
}

void schedule_functor(vvp_ipoint_t fun, unsigned delay)
{
      struct event_s*cur = e_alloc();

      cur->delay = delay;
      cur->fun = fun;
      cur->type = TYPE_PROP;

      schedule_event_(cur);
}

void schedule_assign(vvp_ipoint_t fun, unsigned char val, unsigned delay)
{
      struct event_s*cur = e_alloc();

      cur->delay = delay;
      cur->fun = fun;
      cur->val = val;
      cur->type= TYPE_ASSIGN;

      schedule_event_(cur);
}

void schedule_generic(vvp_gen_event_t obj, unsigned char val, unsigned delay)
{
      struct event_s*cur = e_alloc();

      cur->delay = delay;
      cur->obj = obj;
      cur->val = val;
      cur->type= TYPE_GEN;

      schedule_event_(cur);
}

static unsigned long schedule_time;
unsigned long schedule_simtime(void)
{ return schedule_time; }

void schedule_simulate(void)
{
      schedule_time = 0;

      while (schedule_runnable && sched_list) {

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
			e_free(sync_cur);
		  }


		  schedule_time += cur->delay;
		    //printf("TIME: %u\n", schedule_time);
	    }

	    switch (cur->type) {
		case TYPE_THREAD:
		  vthread_run(cur->thr);
		  e_free(cur);
		  break;

		case TYPE_PROP:
		    //printf("Propagate %p\n", cur->fun);
		  functor_propagate(cur->fun);
		  e_free(cur);
		  break;

		case TYPE_ASSIGN:
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
		  e_free(cur);
		  break;

		case TYPE_GEN:
		  if (cur->obj && cur->obj->run) {
			if (cur->obj->sync_flag == false) {
			      cur->obj->run(cur->obj, cur->val);
			      e_free(cur);

			} else {
			      postpone_sync_event(cur);
			}
		  }
		  break;

	    }

      }
}

/*
 * $Log: schedule.cc,v $
 * Revision 1.12  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.11  2001/07/11 02:27:21  steve
 *  Add support for REadOnlySync and monitors.
 *
 * Revision 1.10  2001/05/30 03:02:35  steve
 *  Propagate strength-values instead of drive strengths.
 *
 * Revision 1.9  2001/05/08 23:32:26  steve
 *  Add to the debugger the ability to view and
 *  break on functors.
 *
 *  Add strengths to functors at compile time,
 *  and Make functors pass their strengths as they
 *  propagate their output.
 *
 * Revision 1.8  2001/05/05 23:51:49  steve
 *  Forward the simulation time for every event.
 *
 * Revision 1.7  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
 *
 * Revision 1.6  2001/04/21 00:34:39  steve
 *  Working %disable and reap handling references from scheduler.
 *
 * Revision 1.5  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.4  2001/03/31 19:00:43  steve
 *  Add VPI support for the simulation time.
 *
 * Revision 1.3  2001/03/19 01:55:38  steve
 *  Add support for the vpiReset sim control.
 *
 * Revision 1.2  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */

