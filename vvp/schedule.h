#ifndef __schedule_H
#define __schedule_H
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
#ident "$Id: schedule.h,v 1.5 2001/05/01 01:09:39 steve Exp $"
#endif

# include  "vthread.h"
# include  "pointers.h"

/*
 * This causes a thread to be scheduled for execution. The schedule
 * puts the event into the event queue after any existing events for a
 * given time step. The delay is a relative time.
 */
extern void schedule_vthread(vthread_t thr, unsigned delay);

/*
 * Create a propagation event. The fun parameter points to the functor
 * to have its output propagated, and the delay is the delay to
 * schedule the propagation.
 */
extern void schedule_functor(vvp_ipoint_t fun, unsigned delay);

/*
 * Create an assignment event. The val passed here will be assigned to
 * the specified input when the delay times out.
 */
extern void schedule_assign(vvp_ipoint_t fun, unsigned char val,
			    unsigned delay);

/*
 * Create an abstract event.
 */

typedef struct vvp_gen_event_s *vvp_gen_event_t;

extern void schedule_generic(vvp_gen_event_t obj, unsigned char val,
			     unsigned delay);

struct vvp_gen_event_s
{
  void (*run)(vvp_gen_event_t obj, unsigned char val);
};

/*
 * This runs the simulator. It runs until all the functors run out or
 * the simulation is otherwise finished.
 */
extern void schedule_simulate(void);

/*
 * Get the current absolue simulation time. This is not used
 * internally by the scheduler (which uses time differences instead)
 * but is used for printouts and stuff.
 */
extern unsigned long schedule_simtime(void);

/*
 * This function is the equivilent of the $finish system task. It
 * tells the simulator that simulation is done, the current thread
 * should be stopped, all remaining events abandoned and the
 * schedule_simulate() function will return.
 *
 * The schedule_finished() function will return true if the
 * schedule_finish() function has been called.
 */
extern void schedule_finish(int rc);
extern bool schedule_finished(void);


/*
 * $Log: schedule.h,v $
 * Revision 1.5  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
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
#endif
