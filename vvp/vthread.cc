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
#ident "$Id: vthread.cc,v 1.1 2001/03/11 00:29:39 steve Exp $"
#endif

# include  "vthread.h"
# include  "codes.h"
# include  "schedule.h"

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

	    bool rc = (cp->opcode)(thr, cp);
	    if (rc == false)
		  return;
      }
}

bool of_ASSIGN(vthread_t thr, vvp_code_t cp)
{
      printf("thread %p: %%assign\n");
      return true;
}

bool of_DELAY(vthread_t thr, vvp_code_t cp)
{
      printf("thread %p: %%delay %u\n", thr, cp->number);
      schedule_vthread(thr, cp->number);
      return false;
}

bool of_END(vthread_t thr, vvp_code_t cp)
{
      printf("thread %p: %%end\n", thr);
      return false;
}

bool of_NOOP(vthread_t thr, vvp_code_t cp)
{
      return true;
}

bool of_SET(vthread_t thr, vvp_code_t cp)
{
      printf("thread %p: %%set\n");
      return true;
}

/*
 * $Log: vthread.cc,v $
 * Revision 1.1  2001/03/11 00:29:39  steve
 *  Add the vvp engine to cvs.
 *
 */

