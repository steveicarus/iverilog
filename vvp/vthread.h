#ifndef __vthread_H
#define __vthread_H
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
#ident "$Id: vthread.h,v 1.11 2003/07/03 20:03:36 steve Exp $"
#endif

/*
 * A vthread is a simulation thread that executes instructions when
 * they are scheduled. This structure contains all the thread specific
 * context needed to run an instruction.
 *
 * Included in a thread are its program counter, local bits and
 * members needed for tracking the thread. The thread runs by fetching
 * instructions from code space, interpreting the instruction, then
 * fetching the next instruction.
 */

typedef struct vthread_s* vthread_t;
typedef struct vvp_code_s*vvp_code_t;

/*
 * This creates a new simulation thread, with the given start
 * address. The generated thread is ready to run, but is not yet
 * scheduled.
 */
extern vthread_t vthread_new(vvp_code_t sa, struct __vpiScope*scope);

/*
 * This function marks the thread as scheduled. It is used only by the
 * schedule_vthread function.
 */
extern void vthread_mark_scheduled(vthread_t thr);

/*
 * Cause this thread to execute instructions until in is put to sleep
 * by executing some sort of delay or wait instruction.
 */
extern void vthread_run(vthread_t thr);

/*
 * This function schedules all the threads in the list to be scheduled
 * for execution with delay 0. The thr pointer is taken to be the head
 * of a list, and all the threads in the list are presumably placed in
 * the list by the %wait instruction.
 */
extern void vthread_schedule_list(vthread_t thr);


/*
 * Return a bit from the thread's bit space. These are used, for
 * example, when a VPI implementation function needs to access the bit
 * space of the thread.
 */
extern unsigned vthread_get_bit(struct vthread_s*thr, unsigned addr);
extern void vthread_put_bit(struct vthread_s*thr, unsigned addr, unsigned bit);

extern double vthread_get_real(struct vthread_s*thr, unsigned addr);
extern void vthread_put_real(struct vthread_s*thr, unsigned addr, double val);

/*
 * $Log: vthread.h,v $
 * Revision 1.11  2003/07/03 20:03:36  steve
 *  Remove the vvp_cpoint_t indirect code pointer.
 *
 * Revision 1.10  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.9  2003/01/26 18:16:22  steve
 *  Add %cvt/ir and %cvt/ri instructions, and support
 *  real values passed as arguments to VPI tasks.
 *
 * Revision 1.8  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2001/05/20 00:45:43  steve
 *  include missing externs on vthread_put_bit.
 *
 * Revision 1.6  2001/05/10 00:26:53  steve
 *  VVP support for memories in expressions,
 *  including general support for thread bit
 *  vectors as system task parameters.
 *  (Stephan Boettcher)
 *
 * Revision 1.5  2001/04/21 00:34:39  steve
 *  Working %disable and reap handling references from scheduler.
 *
 * Revision 1.4  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.3  2001/04/13 03:55:18  steve
 *  More complete reap of all threads.
 *
 * Revision 1.2  2001/03/26 04:00:39  steve
 *  Add the .event statement and the %wait instruction.
 *
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
