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
#ident "$Id: schedule.cc,v 1.8 2001/05/05 23:51:49 steve Exp $"
#endif

# include  "schedule.h"
# include  "functor.h"
# include  "memory.h"
# include  "vthread.h"
# include  <malloc.h>
# include  <assert.h>

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
 * This is the head of the list of pending events.
 */
static struct event_s* list = 0;

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
      if (list == 0) {
	    list = cur;
	    cur->next = 0;
	    return;
      }

      struct event_s*idx = list;
      if (cur->delay < idx->delay) {
	      /* If this new event is earlier then even the first
		 event, then insert it in front. Adjust the delay of
		 the next event, and set the start to me. */
	    idx->delay -= cur->delay;
	    cur->next = idx;
	    list = cur;

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

      while (schedule_runnable && list) {

	      /* Pull the first item off the list. Fixup the last
		 pointer in the next cell, if necessary. */
	    struct event_s*cur = list;
	    list = cur->next;
	    if (cur->last != cur) {
		  assert(list);
		  assert(list->delay == 0);
		  list->last = cur->last;

	    }

	    schedule_time += cur->delay;
	      //printf("TIME: %u\n", schedule_time);

	    switch (cur->type) {
		case TYPE_THREAD:
		  vthread_run(cur->thr);
		  break;

		case TYPE_PROP:
		    //printf("Propagate %p\n", cur->fun);
		  functor_propagate(cur->fun);
		  break;

		case TYPE_ASSIGN:
		  functor_set(cur->fun, cur->val);
		  break;

		case TYPE_GEN:
		  if (cur->obj && cur->obj->run)
		    cur->obj->run(cur->obj, cur->val);
		  break;

	    }

	    e_free(cur);
      }
}

/*
 * $Log: schedule.cc,v $
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

