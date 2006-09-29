#ifndef __schedule_H
#define __schedule_H
/*
 * Copyright (c) 2001-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: schedule.h,v 1.28 2006/09/29 01:24:34 steve Exp $"
#endif

# include  "vthread.h"
# include  "pointers.h"
# include  "vvp_net.h"
# include  "memory.h"

/*
 * This causes a thread to be scheduled for execution. The schedule
 * puts the event into the event queue after any existing events for a
 * given time step. The delay is a relative time.
 *
 * If the delay is zero, the push_flag can be used to force the event
 * to the front of the queue. %fork uses this to get the thread
 * execution ahead of non-blocking assignments.
 */
extern void schedule_vthread(vthread_t thr, vvp_time64_t delay,
			     bool push_flag =false);

/*
 * Create an assignment event. The val passed here will be assigned to
 * the specified input when the delay times out. This is scheduled
 * like a non-blocking assignment. This is in fact mostly used to
 * implement the non-blocking assignment.
 */
extern void schedule_assign_vector(vvp_net_ptr_t ptr,
				   unsigned base, unsigned vwid,
				   const vvp_vector4_t&val,
				   vvp_time64_t  delay);

extern void schedule_assign_vector(vvp_net_ptr_t ptr,
				   const vvp_vector4_t&val,
				   vvp_time64_t  delay);

extern void schedule_assign_memory_word(vvp_memory_t mem,
					unsigned word_address,
					unsigned off,
					vvp_vector4_t val,
					vvp_time64_t delay);
/*
 * This is very similar to schedule_assign_vector, but generates an
 * event in the active queue. It is used at link time to set an initial
 * value (a compile time constant) to the input of a functor. This
 * creates an event in the active queue.
 */
extern void schedule_set_vector(vvp_net_ptr_t ptr, vvp_vector4_t val);
extern void schedule_set_vector(vvp_net_ptr_t ptr, vvp_vector8_t val);
extern void schedule_set_vector(vvp_net_ptr_t ptr, double val);


/*
 * Create a generic event. This supports scheduled events that are not
 * any of the specific types above. The vvp_get_event_t points to a
 * function to be executed when the scheduler gets to the event. It is
 * up to the user to allocate/free the vvp_get_event_s object. The
 * object is never referenced by the scheduler after the run method is
 * called.
 *
 * The sync_flag is true if this is intended to be a sync event. These
 * are placed in the stratified event queue after nb assignes. If the
 * ro_flag is true as well, then it is a Read-only sync event, with
 * all that means.
 *
 * If the sync_flag is false, then the event is ACTIVE, and the
 * ro_flag is ignored.
 */

typedef struct vvp_gen_event_s *vvp_gen_event_t;

extern void schedule_generic(vvp_gen_event_t obj, vvp_time64_t delay,
			     bool sync_flag, bool ro_flag =true);

struct vvp_gen_event_s
{
      virtual ~vvp_gen_event_s() =0;
      virtual void run_run() =0;
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
extern void schedule_stop(int rc);
extern bool schedule_finished(void);
extern bool schedule_stopped(void);

/*
 * The scheduler calls this function to process stop events. When this
 * function returns, the simulation resumes.
 */
extern void stop_handler(int rc);

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
 * Revision 1.28  2006/09/29 01:24:34  steve
 *  rwsync callback fixes from Ben Staveley (with modifications.)
 *
 * Revision 1.27  2006/02/02 02:44:00  steve
 *  Allow part selects of memory words in l-values.
 *
 * Revision 1.26  2005/09/20 18:34:02  steve
 *  Clean up compiler warnings.
 *
 * Revision 1.25  2005/07/06 04:29:25  steve
 *  Implement real valued signals and arith nodes.
 *
 * Revision 1.24  2005/06/22 18:30:12  steve
 *  Inline more simple stuff, and more vector4_t by const reference for performance.
 *
 * Revision 1.23  2005/06/09 05:04:45  steve
 *  Support UDP initial values.
 *
 * Revision 1.22  2005/06/02 16:02:11  steve
 *  Add support for notif0/1 gates.
 *  Make delay nodes support inertial delay.
 *  Add the %force/link instruction.
 *
 * Revision 1.21  2005/05/07 03:15:42  steve
 *  Implement non-blocking part assign.
 *
 * Revision 1.20  2005/03/06 17:07:48  steve
 *  Non blocking assign to memory words.
 *
 * Revision 1.19  2005/02/12 03:26:14  steve
 *  Support scheduling vvp_vector8_t objects.
 *
 * Revision 1.18  2005/01/29 17:53:25  steve
 *  Use scheduler to initialize constant functor inputs.
 *
 * Revision 1.17  2004/12/11 02:31:30  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.16  2003/09/09 00:56:45  steve
 *  Reimpelement scheduler to divide nonblocking assign queue out.
 *
 * Revision 1.15  2003/02/22 02:52:06  steve
 *  Check for stopped flag in certain strategic points.
 */
#endif
