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
#ident "$Id: vthread.cc,v 1.5 2001/03/19 01:55:38 steve Exp $"
#endif

# include  "vthread.h"
# include  "codes.h"
# include  "schedule.h"
# include  "functor.h"
# include  "vpi_priv.h"
# include  <assert.h>

struct vthread_s {
	/* This is the program counter. */
      unsigned long pc;
};

/*
 * Create a new thread with the given start address.
 */
vthread_t v_newthread(unsigned long pc)
{
      vthread_t thr = new struct vthread_s;
      thr->pc = pc;

      return thr;
}


/*
 * This function runs a thread by fetching an instruction,
 * incrementing the PC, and executing the instruction.
 */
void vthread_run(vthread_t thr)
{
      for (;;) {
	    vvp_code_t cp = codespace_index(thr->pc);
	    thr->pc += 1;

	    assert(cp->opcode);

	      /* Run the opcode implementation. If the execution of
		 the opcode returns false, then the thread is meant to
		 be paused, so break out of the loop. */
	    bool rc = (cp->opcode)(thr, cp);
	    if (rc == false)
		  return;
      }
}

bool of_ASSIGN(vthread_t thr, vvp_code_t cp)
{
	//printf("thread %p: %%assign\n", thr);

      unsigned char bit_val = 3;
      if ((cp->bit_idx2 & ~0x3) == 0x0) {
	    bit_val = cp->bit_idx2&3;

      } else {
	    printf("XXXX bit_idx out of range?\n");
      }

      schedule_assign(cp->iptr, bit_val, cp->bit_idx1);
      return true;
}

bool of_DELAY(vthread_t thr, vvp_code_t cp)
{
	//printf("thread %p: %%delay %lu\n", thr, cp->number);
      schedule_vthread(thr, cp->number);
      return false;
}

bool of_END(vthread_t thr, vvp_code_t cp)
{
	//printf("thread %p: %%end\n", thr);
      return false;
}

bool of_NOOP(vthread_t thr, vvp_code_t cp)
{
      return true;
}

bool of_SET(vthread_t thr, vvp_code_t cp)
{
	//printf("thread %p: %%set %u, %u\n", thr, cp->iptr, cp->bit_idx1);

      unsigned char bit_val = 3;
      if ((cp->bit_idx1 & ~0x3) == 0x0) {
	    bit_val = cp->bit_idx1&3;

      } else {
	    printf("XXXX bit_idx out of range?\n");
      }

      functor_set(cp->iptr, bit_val);

      return true;
}

bool of_VPI_CALL(vthread_t thr, vvp_code_t cp)
{
	// printf("thread %p: %%vpi_call\n", thr);
      vpip_execute_vpi_call(cp->handle);
      return schedule_finished()? false : true;
}

/*
 * $Log: vthread.cc,v $
 * Revision 1.5  2001/03/19 01:55:38  steve
 *  Add support for the vpiReset sim control.
 *
 * Revision 1.4  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 * Revision 1.3  2001/03/11 23:06:49  steve
 *  Compact the vvp_code_s structure.
 *
 * Revision 1.2  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */

