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
#ifdef HAVE_CVS_IDENT
#ident "$Id: schedule.h,v 1.13 2003/02/09 23:33:26 steve Exp $"
#endif

# include  "vthread.h"
# include  "pointers.h"

/*
 * This causes a thread to be scheduled for execution. The schedule
 * puts the event into the event queue after any existing events for a
 * given time step. The delay is a relative time.
 *
 * If the delay is zero, the push_flag can be used to force the event
 * to the front of the queue. %fork uses this to get the thread
 * execution ahead of non-blocking assignments.
 */
extern void schedule_vthread(vthread_t thr, unsigned delay,
			     bool push_flag =false);

/*
 * Create an assignment event. The val passed here will be assigned to
 * the specified input when the delay times out.
 */
extern void schedule_assign(vvp_ipoint_t fun, unsigned char val,
			    unsigned delay);


/*
 * Create a generic event. This supports scheduled events that are not
 * any of the specific types above. The vvp_get_event_t points to a
 * function to be executed when the scheduler gets to the event. It is
 * up to the user to allocate/free the vvp_get_event_s object. The
 * object is never referenced by the scheduler after the run method is
 * called.
 */

typedef struct vvp_gen_event_s *vvp_gen_event_t;

extern void schedule_generic(vvp_gen_event_t obj, unsigned char val,
			     unsigned delay);

struct vvp_gen_event_s
{
      void (*run)(vvp_gen_event_t obj, unsigned char val);
      bool sync_flag;
};

/*
 * This runs the simulator. It runs until all the functors run out or
 * the simulation is otherwise finished.
 */
extern void schedule_simulate(void);

/*
 * Get the current absolute simulation time. This is not used
 * internally by the scheduler (which uses time differences instead)
 * but is used for printouts and stuff.
 */
extern vvp_time64_t schedule_simtime(void);

/*
 * This function is the equivalent of the $finish system task. It
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
 * These are event counters for the sake of performance measurements.
 */
extern unsigned long count_assign_events;
extern unsigned long count_gen_events;
extern unsigned long count_prop_events;
extern unsigned long count_thread_events;
extern unsigned long count_event_pool;

/*
 * $Log: schedule.h,v $
 * Revision 1.13  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.12  2003/01/06 23:57:26  steve
 *  Schedule wait lists of threads as a single event,
 *  to save on events. Also, improve efficiency of
 *  event_s allocation. Add some event statistics to
 *  get an idea where performance is really going.
 *
 * Revision 1.11  2002/08/12 01:35:08  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.10  2002/05/12 23:44:41  steve
 *  task calls and forks push the thread event in the queue.
 *
 * Revision 1.9  2002/04/20 04:33:23  steve
 *  Support specified times in cbReadOnlySync, and
 *  add support for cbReadWriteSync.
 *  Keep simulation time in a 64bit number.
 *
 * Revision 1.8  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.7  2001/07/11 02:27:21  steve
 *  Add support for REadOnlySync and monitors.
 *
 * Revision 1.6  2001/05/02 01:38:13  steve
 *  Describe a generic event a bit.
 *
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
