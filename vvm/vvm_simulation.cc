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
#ident "$Id: vvm_simulation.cc,v 1.7 1999/10/28 00:47:25 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_thread.h"
# include  "vpi_priv.h"
# include  <assert.h>

vvm_simulation::vvm_simulation()
{
}

vvm_simulation::~vvm_simulation()
{
}

void vvm_simulation::monitor_event(vvm_event*)
{
      assert(0);
}

void vvm_simulation::insert_event(unsigned long delay, vvm_event*event)
{
      event->event_ = vpip_sim_insert_event(delay, event,
					    vvm_event::callback_, 0);
}

void vvm_simulation::active_event(vvm_event*event)
{
      event->event_ = vpip_sim_insert_event(0, event,
					    vvm_event::callback_, 0);
}

/*
 * This function takes a list of simulation cycles and runs the
 * simulation. It consumes the list as the events are executes,
 * eventually deleting everything.
 */
void vvm_simulation::run()
{
      vpip_simulation_run();
}

class delay_event : public vvm_event {

    public:
      delay_event(vvm_thread*thr) : thr_(thr) { }
      void event_function() { while (thr_->go()) /* empty */; }
    private:
      vvm_thread*thr_;
};


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
 * Revision 1.7  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
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

