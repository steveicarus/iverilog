#ifndef __vthread_H
#define __vthread_H
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
#ident "$Id: vthread.h,v 1.4 2001/04/18 04:21:23 steve Exp $"
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

typedef struct vthread_s*vthread_t;

/*
 * This creates a new simulation thread, with the given start
 * address. The generated thread is ready to run, but is not yet
 * scheduled.
 */
extern vthread_t vthread_new(unsigned long sa, struct __vpiScope*scope);

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
 * $Log: vthread.h,v $
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
