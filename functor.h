#ifndef __functor_H
#define __functor_H
/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: functor.h,v 1.6 1999/12/30 04:19:12 steve Exp $"
#endif

/*
 * The functor is an object that can be applied to a design to
 * transform it. This is different from the target_t, which can only
 * scan the design but not transform it in any way.
 */

class Design;
class NetNet;
class NetProcTop;

struct functor_t {
      virtual ~functor_t();

	/* Signals are scanned first. This is called once for each
	   signal in the design. */
      virtual void signal(class Design*des, class NetNet*);

	/* This method is called for each process in the design. */
      virtual void process(class Design*des, class NetProcTop*);

	/* This method is called for each structural adder. */
      virtual void lpm_add_sub(class Design*des, class NetAddSub*);

	/* This method is called for each structural constant. */
      virtual void lpm_const(class Design*des, class NetConst*);

	/* This method is called for each FF in the design. */
      virtual void lpm_ff(class Design*des, class NetFF*);

	/* Handle LPM combinational logic devices. */
      virtual void lpm_logic(class Design*des, class NetLogic*);
};

struct proc_match_t {
      virtual ~proc_match_t();

      virtual int assign(class NetAssign*);
      virtual int assign_mem(class NetAssignMem*);
      virtual int condit(class NetCondit*);
      virtual int pevent(class NetPEvent*);
};


/*
 * $Log: functor.h,v $
 * Revision 1.6  1999/12/30 04:19:12  steve
 *  Propogate constant 0 in low bits of adders.
 *
 * Revision 1.5  1999/12/17 06:18:16  steve
 *  Rewrite the cprop functor to use the functor_t interface.
 *
 * Revision 1.4  1999/12/05 02:24:08  steve
 *  Synthesize LPM_RAM_DQ for writes into memories.
 *
 * Revision 1.3  1999/12/01 06:06:16  steve
 *  Redo synth to use match_proc_t scanner.
 *
 * Revision 1.2  1999/11/01 02:07:40  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.1  1999/07/17 22:01:13  steve
 *  Add the functor interface for functor transforms.
 *
 */
#endif
