#ifndef __vvm_thread_H
#define __vvm_thread_H
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vvm_thread.h,v 1.8 2000/04/15 01:44:59 steve Exp $"
#endif

# include  "vvm.h"

/*
 * A vvm_thread isn't really a thread in the POSIX sense, but a
 * representation of the verilog thread. It is implemented as a state
 * machine that performs an action, and possibly changes state, every
 * time the go method is called. The events and delays that cause a
 * thread to block arrange for the go method to be called in the
 * future.
 *
 * THREAD STEPS
 * The basic blocks of an executing thread are implemented as C/C++
 * functions that take a vvm_thread pointer and return a boolean. The
 * thread keeps a pointer to the function that is the current step. It
 * is the responsibility of each step to write into the step_ member
 * the pointer to the next step, and that is how things like branching
 * and looping work.
 *
 * A thread executes by calling the current step function. When the
 * step function returns, it uses the bolean return code to tell the
 * scheduler whether the next step should be executed. Thus, a thread
 * can give other threads a chance to execute by returning false.
 *
 * CALLING CONVENTION
 * There are a few members in the vvm_thread for supporting calling
 * other threads. This is like a function call in other languages, as
 * the calling thread blocks until the called thread(s) terminate. It
 * is also like functions in that thread calls nest.
 *
 * A thread is called by creating a new vvm_thread and saving a
 * pointer to the new thread in the callee_ member of the calling
 * thread. The called thread gets its back_ pointer set to that of the
 * calling thread. The new thread is then activated with a call to its
 * thread_yield() method, and the caller thread pauses by returning
 * from its step function with the value "false".
 *
 * When the called thread is ready to terminate, it uses its back_
 * pointer to find the calling thread, and activates it with the
 * thread_yield() method. Then, the called thread finishes by
 * returning false from its step method.
 *
 * When the caller resumes executing, it knows that the called thread
 * is done, so it uses the callee_ pointer to delete the now finished
 * thread, and the process is finished.
 */

class vvm_sync;
class vvm_thread {

    public:
      explicit vvm_thread();
      ~vvm_thread();

      void thread_yield(unsigned long delay =0);

	// This method executes a setp of the thread. The engine will
	// continue to call go as long as it returns true. The thread
	// will return false if it is ready to give up the CPU.
      bool go();
      bool (*step_)(vvm_thread*);

	// These members are used to handle task invocations.
      vvm_thread*callee_;
      vvm_thread*back_;

	// The sync class uses this to list all the threads blocked on it.
      vvm_sync*sync_back_;
      vvm_thread*sync_next_;
};

/*
 * $Log: vvm_thread.h,v $
 * Revision 1.8  2000/04/15 01:44:59  steve
 *  Document the calling convention.
 *
 * Revision 1.7  2000/04/14 23:31:53  steve
 *  No more class derivation from vvm_thread.
 *
 * Revision 1.6  2000/04/12 01:53:07  steve
 *  Multiple thread can block on an event.
 *
 * Revision 1.5  2000/02/23 02:56:57  steve
 *  Macintosh compilers do not support ident.
 *
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
