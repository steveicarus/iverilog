#ifndef __vvm_thread_H
#define __vvm_thread_H
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_thread.h,v 1.4 2000/01/06 05:56:23 steve Exp $"
#endif

# include  "vvm.h"

/*
 * A vvm_thread isn't really a thread in the POSIX sense, but a
 * representation of the verilog thread. It is implemented as a state
 * machine that performs an action, and possibly changes state, every
 * time the go method is called. The events and delays that cause a
 * thread to block arrange for the go method to be called in the
 * future.
 */

class vvm_thread {

    public:
      explicit vvm_thread();
      virtual ~vvm_thread();

      void thread_yield(unsigned long delay =0);

	// This method executes a setp of the thread. The engine will
	// continue to call go as long as it returns true. The thread
	// will return false if it is ready to give up the CPU.
      virtual bool go() =0;

};

/*
 * $Log: vvm_thread.h,v $
 * Revision 1.4  2000/01/06 05:56:23  steve
 *  Cleanup and some asserts.
 *
 * Revision 1.3  1999/12/12 19:47:54  steve
 *  Remove the useless vvm_simulation class.
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
#endif
