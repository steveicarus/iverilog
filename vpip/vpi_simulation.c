/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_simulation.c,v 1.3 2002/08/12 01:35:06 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <stdarg.h>
# include  <assert.h>

struct vpip_event {
      void*user_data;
      void (*sim_fun)(void*);
      struct vpip_event*next;
};

static struct vpip_simulation vpip_simulation_obj;

struct vpip_simulation *vpip_get_simulation_obj(void)
{
    return &vpip_simulation_obj;
}



extern void vpi_sim_vcontrol(int func, va_list ap)
{
      switch (func) {
	  case vpiFinish:
	    vpip_simulation_obj.going_flag = 0;
	    break;
      }
}

/*
 * The simulation event queue is a list of simulation cycles that in
 * turn contain lists of simulation events. The simulation cycle
 * represents the happening at some simulation time.
 */
struct vpip_simulation_cycle {
      unsigned long delay;

      struct vpip_simulation_cycle*next;
      struct vpip_simulation_cycle*prev;

      struct vpip_event*event_list;
      struct vpip_event*event_last;
      struct vpip_event*nonblock_list;
      struct vpip_event*nonblock_last;
};

void vpip_init_simulation()
{
      struct vpip_simulation_cycle*cur;

      vpip_make_time_var(&vpip_simulation_obj.sim_time, "$time");
      vpip_simulation_obj.read_sync_list = 0;

	/* Allocate a header cell for the simulation event list. */
      cur = calloc(1, sizeof(struct vpip_simulation_cycle));
      cur->delay = 0;
      cur->next = cur->prev = cur;
      vpip_simulation_obj.sim = cur;
      vpip_simulation_obj.time_precision = 0;

      vpi_mcd_init();
}

void vpip_time_scale(int precision)
{
      vpip_simulation_obj.time_precision = precision;
}

vpiHandle vpip_sim_time()
{
      return &vpip_simulation_obj.sim_time.base;
}

struct vpip_event* vpip_sim_insert_event(unsigned long delay,
					 void*user_data,
					 void (*sim_fun)(void*),
					 int nonblock_flag)
{
      struct vpip_event*event;
      struct vpip_simulation_cycle*cur;

      event = calloc(1, sizeof(struct vpip_event));
      event->user_data = user_data;
      event->sim_fun = sim_fun;

      cur = vpip_simulation_obj.sim->next;
      while ((cur != vpip_simulation_obj.sim) && (cur->delay < delay)) {
	    delay -= cur->delay;
	    cur = cur->next;
      }

	/* If there is no cycle cell for the specified time, create one. */
      if ((cur == vpip_simulation_obj.sim) || (cur->delay > delay)) {
	    struct vpip_simulation_cycle*cell
		  = calloc(1,sizeof(struct vpip_simulation_cycle));
	    cell->delay = delay;
	    if (cur != vpip_simulation_obj.sim)
		  cur->delay -= delay;
	    cell->next = cur;
	    cell->prev = cur->prev;
	    cell->next->prev = cell;
	    cell->prev->next = cell;
	    cur = cell;
      }

	/* Put the event into the end of the cycle list. */
      event->next = 0;
      if (nonblock_flag) {
	    if (cur->nonblock_list == 0) {
		  cur->nonblock_list = cur->nonblock_last = event;

	    } else {
		  cur->nonblock_last->next = event;
		  cur->nonblock_last = event;
	    }
      } else {
	    if (cur->event_list == 0) {
		  cur->event_list = cur->event_last = event;

	    } else {
		  cur->event_last->next = event;
		  cur->event_last = event;
	    }
      }

      return event;
}

void vpip_sim_cancel_event(struct vpip_event*ev)
{
      assert(0);
}

int vpip_finished()
{
      return ! vpip_simulation_obj.going_flag;
}

void vpip_simulation_run()
{
      vpip_simulation_obj.sim_time.time = 0;
      vpip_simulation_obj.going_flag = !0;

      while (vpip_simulation_obj.going_flag) {
	    struct vpip_simulation_cycle*sim = vpip_simulation_obj.sim;

	      /* Step the time forward to the next time cycle. */
	    vpip_simulation_obj.sim_time.time += sim->delay;
	    sim->delay = 0;

	    while (vpip_simulation_obj.going_flag) {
		  struct vpip_event*active = sim->event_list;
		  sim->event_list = 0;
		  sim->event_last = 0;

		  if (active == 0) {
			active = sim->nonblock_list;
			sim->nonblock_list = 0;
			sim->nonblock_last = 0;
		  }

		  if (active == 0)
			break;

		  while (active && vpip_simulation_obj.going_flag) {
			struct vpip_event*cur = active;
			active = cur->next;
			(cur->sim_fun)(cur->user_data);
			free(cur);
		  }
	    }

	    if (! vpip_simulation_obj.going_flag)
		  break;

	    { struct vpip_simulation_cycle*next = sim->next;
	      if (next == sim) {
		    vpip_simulation_obj.going_flag = 0;
		    break;
	      }
	      sim->next->prev = sim->prev;
	      sim->prev->next = sim->next;
	      free(sim);
	      vpip_simulation_obj.sim = next;
	    }

      }
}

/*
 * $Log: vpi_simulation.c,v $
 * Revision 1.3  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/06/12 03:53:10  steve
 *  Change the VPI call process so that loaded .vpi modules
 *  use a function table instead of implicit binding.
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.5  2000/10/06 23:11:39  steve
 *  Replace data references with function calls. (Venkat)
 *
 * Revision 1.4  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.3  2000/07/26 03:53:12  steve
 *  Make simulation precision available to VPI.
 *
 * Revision 1.2  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

