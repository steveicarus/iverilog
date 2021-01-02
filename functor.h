#ifndef IVL_functor_H
#define IVL_functor_H
/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * The functor is an object that can be applied to a design to
 * transform it. This is different from the target_t, which can only
 * scan the design but not transform it in any way.
 *
 * When a functor it scanning a process, signal or node, the functor
 * is free to manipulate the list by deleting items, including the
 * node being scanned. The Design class scanner knows how to handle
 * the situation. However, if objects are added to the netlist, there
 * is no guarantee that object will be scanned unless the functor is
 * rerun.
 */

class Design;
class NetNet;
class NetProcTop;

struct functor_t {
      virtual ~functor_t();

	/* Events are scanned here. */
      virtual void event(Design*des, class NetEvent*);

	/* This is called once for each signal in the design. */
      virtual void signal(Design*des, NetNet*);

	/* This method is called for each process in the design. */
      virtual void process(Design*des, NetProcTop*);

	/* This method is called for each structural abs(). */
      virtual void lpm_abs(Design*des, class NetAbs*);

	/* This method is called for each structural adder. */
      virtual void lpm_add_sub(Design*des, class NetAddSub*);

	/* This method is called for each structural comparator. */
      virtual void lpm_compare(Design*des, const class NetCompare*);

	/* This method is called for each structural concatenation. */
      virtual void lpm_concat(Design*des, class NetConcat*);

	/* This method is called for each structural constant. */
      virtual void lpm_const(Design*des, class NetConst*);

	/* This method is called for each structural constant. */
      virtual void lpm_divide(Design*des, class NetDivide*);

	/* Constant literals. */
      virtual void lpm_literal(Design*des, class NetLiteral*);

	/* This method is called for each structural constant. */
      virtual void lpm_modulo(Design*des, class NetModulo*);

	/* This method is called for each FF in the design. */
      virtual void lpm_ff(Design*des, class NetFF*);

	/* This method is called for each LATCH in the design. */
      virtual void lpm_latch(Design*des, class NetLatch*);

	/* Handle LPM combinational logic devices. */
      virtual void lpm_logic(Design*des, class NetLogic*);

	/* This method is called for each multiplier. */
      virtual void lpm_mult(Design*des, class NetMult*);

	/* This method is called for each MUX. */
      virtual void lpm_mux(Design*des, class NetMux*);

      virtual void lpm_part_select(Design*des, class NetPartSelect*);

	/* This method is called for each power. */
      virtual void lpm_pow(Design*des, class NetPow*);

	/* This method is called for each part substitute. */
      virtual void lpm_substitute(Design*des, class NetSubstitute*);

	/* This method is called for each unary reduction gate. */
      virtual void lpm_ureduce(Design*des, class NetUReduce*);

      virtual void sign_extend(Design*des, class NetSignExtend*);
};

struct proc_match_t {
      virtual ~proc_match_t();

      virtual int assign(class NetAssign*);
      virtual int assign_nb(class NetAssignNB*);
      virtual int condit(class NetCondit*);
      virtual int event_wait(class NetEvWait*);
      virtual int block(class NetBlock*);
};

#endif /* IVL_functor_H */
