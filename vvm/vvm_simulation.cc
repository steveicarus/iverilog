/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_simulation.cc,v 1.6 1999/10/06 01:28:18 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_thread.h"
# include  <assert.h>

/*
 * The state of the simulation is stored as a list of simulation
 * times. Each simulation time contains the delay to get to it, and
 * also a list of events that are to execute here.
 *
 * The "events" member points to a list of inactive events to be made
 * active as a group. Update events go into this list.
 *
 * The "nonblock" member points to a list of events to be executed
 * only after ordinary events run out. This is intended to support the
 * semantics of nonblocking assignment.
 */
struct vvm_simulation_cycle {
      unsigned long delay;

      struct vvm_simulation_cycle*next;
      struct vvm_simulation_cycle*prev;

      struct vvm_event*event_list;
      struct vvm_event*event_last;
      struct vvm_event*nonblock_list;
      struct vvm_event*nonblock_last;

      vvm_simulation_cycle()
      : event_list(0), event_last(0), nonblock_list(0),
	nonblock_last(0)
      { }
};

vvm_simulation::vvm_simulation()
{
      sim_ = new vvm_simulation_cycle;
      sim_->delay = 0;
      sim_->next = sim_->prev = sim_;
      mon_ = 0;
}

vvm_simulation::~vvm_simulation()
{
}

void vvm_simulation::insert_event(unsigned long delay, vvm_event*event)
{
      vvm_simulation_cycle*cur = sim_->next;

      while ((cur != sim_) && (cur->delay < delay)) {
	    delay -= cur->delay;
	    cur = cur->next;
      }


      if ((cur == sim_) || (cur->delay > delay)) {
	    vvm_simulation_cycle*cell = new vvm_simulation_cycle;
	    cell->delay = delay;
	    if (cur != sim_)
		  cur->delay -= delay;
	    cell->next = cur;
	    cell->prev = cur->prev;
	    cell->next->prev = cell;
	    cell->prev->next = cell;
	    cur = cell;
      }

      event->next_ = 0;
      if (cur->event_list == 0) {
	    cur->event_list = cur->event_last = event;

      } else {
	    cur->event_last->next_ = event;
	    cur->event_last = event;
      }
}

void vvm_simulation::monitor_event(vvm_event*event)
{
      mon_ = event;
}

void vvm_simulation::active_event(vvm_event*event)
{
      event->next_ = 0;
      if (sim_->event_list == 0) {
	    sim_->event_list = sim_->event_last = event;

      } else {
	    sim_->event_last->next_ = event;
	    sim_->event_last = event;
      }
}

/*
 * This function takes a list of simulation cycles and runs the
 * simulation. It consumes the list as the events are executes,
 * eventually deleting everything.
 */
void vvm_simulation::run()
{
      assert(sim_);
      time_ = 0;
      going_ = true;

      while  (going_) {

	      /* Step the time forward to the cycle I am about to
		 execute. */
	    time_ += sim_->delay;
	    sim_->delay = 0;

	    while (going_) {
		    /* Look for some events to make active. If the
		       main event list is empty, then activate the
		       nonblock list. */
		  vvm_event*active = sim_->event_list;
		  sim_->event_list = 0;
		  sim_->event_last = 0;

		  if (active == 0) {
			active = sim_->nonblock_list;
			sim_->nonblock_list = 0;
			sim_->nonblock_last = 0;
		  }

		    /* Oops, no events left. Break out of this time cycle. */
		  if (active == 0)
			break;

		  while (active && going_) {
			vvm_event*cur = active;
			active = cur->next_;
			cur->event_function();
			delete cur;
		  }
	    }

	      /* If the simulation was stopped by one of the events,
		 then break out of the loop before doing any monitor
		 events, and before clearing the current time. */
	    if (!going_)
		  break;

	      /* XXXX Execute monitor events here. */
	    if (mon_) {
		  mon_->event_function();
		  mon_ = 0;
	    }

	      /* The time cycle is done, delete it from the list and
		 step to the next time. */
	    struct vvm_simulation_cycle*next = sim_->next;
	    if (next == sim_) {
		  going_ = false;
		  break;
	    }

	    sim_->next->prev = sim_->prev;
	    sim_->prev->next = sim_->next;
	    delete sim_;
	    sim_ = next;
      }
}

class delay_event : public vvm_event {

    public:
      delay_event(vvm_thread*thr) : thr_(thr) { }
      void event_function() { while (thr_->go()) /* empty */; }
    private:
      vvm_thread*thr_;
};

void vvm_simulation::s_finish()
{
      going_ = false;
}

bool vvm_simulation::finished() const
{
      return !going_;
}

void vvm_simulation::thread_delay(unsigned long delay, vvm_thread*thr)
{
      delay_event*ev = new delay_event(thr);
      insert_event(delay, ev);
}

void vvm_simulation::thread_active(vvm_thread*thr)
{
      delay_event*ev = new delay_event(thr);
      active_event(ev);
}


/*
 * $Log: vvm_simulation.cc,v $
 * Revision 1.6  1999/10/06 01:28:18  steve
 *  The $finish task should work immediately.
 *
 * Revision 1.5  1999/09/29 02:53:33  steve
 *  Useless assertion.
 *
 * Revision 1.4  1999/06/19 03:31:33  steve
 *  End run if events run out.
 *
 * Revision 1.3  1999/06/07 03:40:03  steve
 *  Allow 0 delays for things like thread yield.
 *
 * Revision 1.2  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */

