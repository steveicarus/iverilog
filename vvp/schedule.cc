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
#ident "$Id: schedule.cc,v 1.3 2001/03/19 01:55:38 steve Exp $"
#endif

# include  "schedule.h"
# include  "functor.h"
# include  "vthread.h"
# include  <malloc.h>
# include  <assert.h>

struct event_s {
      unsigned delay;

      union {
	    vthread_t thr;
	    vvp_ipoint_t fun;
      };
      unsigned val  :2;
      unsigned type :2;

      struct event_s*next;
      struct event_s*last;
};
const unsigned TYPE_THREAD = 0;
const unsigned TYPE_PROP   = 1;
const unsigned TYPE_ASSIGN = 2;

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

static void schedule_event_(struct event_s*cur)
{
      cur->last = cur;

      if (list == 0) {
	    list = cur;
	    cur->next = 0;
	    return;
      }

      struct event_s*idx = list;
      if (cur->delay < idx->delay) {
	    idx->delay -= cur->delay;
	    cur->next = idx;
	    list = cur;

      } else {
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
		  cur->last = cur;
		  cur->next = idx;
		  prev->next = cur;

	    } else {
		  assert(cur->delay == idx->delay);
		  cur->delay = 0;
		  cur->last = cur;
		  cur->next = idx->last->next;
		  idx->last->next = cur;
		  idx->last = cur;
	    }
      }
}

void schedule_vthread(vthread_t thr, unsigned delay)
{
      struct event_s*cur = (struct event_s*)
	    calloc(1, sizeof(struct event_s));

      cur->delay = delay;
      cur->thr = thr;
      cur->type = TYPE_THREAD;

      schedule_event_(cur);
}

void schedule_functor(vvp_ipoint_t fun, unsigned delay)
{
      struct event_s*cur = (struct event_s*)
	    calloc(1, sizeof(struct event_s));

      cur->delay = delay;
      cur->fun = fun;
      cur->type = TYPE_PROP;

      schedule_event_(cur);
}

void schedule_assign(vvp_ipoint_t fun, unsigned char val, unsigned delay)
{
      struct event_s*cur = (struct event_s*)
	    calloc(1, sizeof(struct event_s));

      cur->delay = delay;
      cur->fun = fun;
      cur->val = val;
      cur->type= TYPE_ASSIGN;

      schedule_event_(cur);

}

static unsigned long schedule_time;

void schedule_simulate(void)
{
      schedule_time = 0;

      while (schedule_runnable && list) {

	      /* Pull the first item off the list. Fixup the last
		 pointer in the next cell, if necessary. */
	    struct event_s*cur = list;
	    list = cur->next;
	    if (cur->last != cur) {
		  assert(list->delay == 0);
		  list->last = cur->last;

	    } else {
		  schedule_time += cur->delay;
		    //printf("TIME: %u\n", schedule_time);
	    }

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

	    }

	    free(cur);
      }
}

/*
 * $Log: schedule.cc,v $
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

