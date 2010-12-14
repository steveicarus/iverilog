#ifndef __vthread_H
#define __vthread_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

#endif
