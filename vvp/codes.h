#ifndef __codes_H
#define __codes_H
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
#ident "$Id: codes.h,v 1.65 2004/05/19 03:26:24 steve Exp $"
#endif


# include  "pointers.h"
# include  "memory.h"
# include  "vthread.h"

typedef bool (*vvp_code_fun)(vthread_t thr, vvp_code_t code);

/*
 * These functions are implementations of executable op-codes. The
 * implementation lives in the vthread.cc file so that they have
 * access to the thread context.
 */
extern bool of_ADD(vthread_t thr, vvp_code_t code);
extern bool of_ADD_WR(vthread_t thr, vvp_code_t code);
extern bool of_ADDI(vthread_t thr, vvp_code_t code);
extern bool of_AND(vthread_t thr, vvp_code_t code);
extern bool of_ANDR(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_D(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_MEM(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_V0(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_WR(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_X0(vthread_t thr, vvp_code_t code);
extern bool of_BLEND(vthread_t thr, vvp_code_t code);
extern bool of_BREAKPOINT(vthread_t thr, vvp_code_t code);
extern bool of_CASSIGN(vthread_t thr, vvp_code_t code);
extern bool of_CMPIU(vthread_t thr, vvp_code_t code);
extern bool of_CMPS(vthread_t thr, vvp_code_t code);
extern bool of_CMPU(vthread_t thr, vvp_code_t code);
extern bool of_CMPWR(vthread_t thr, vvp_code_t code);
extern bool of_CMPX(vthread_t thr, vvp_code_t code);
extern bool of_CMPZ(vthread_t thr, vvp_code_t code);
extern bool of_CVT_IR(vthread_t thr, vvp_code_t code);
extern bool of_CVT_RI(vthread_t thr, vvp_code_t code);
extern bool of_CVT_VR(vthread_t thr, vvp_code_t code);
extern bool of_DEASSIGN(vthread_t thr, vvp_code_t code);
extern bool of_DELAY(vthread_t thr, vvp_code_t code);
extern bool of_DELAYX(vthread_t thr, vvp_code_t code);
extern bool of_DISABLE(vthread_t thr, vvp_code_t code);
extern bool of_DIV(vthread_t thr, vvp_code_t code);
extern bool of_DIV_S(vthread_t thr, vvp_code_t code);
extern bool of_DIV_WR(vthread_t thr, vvp_code_t code);
extern bool of_END(vthread_t thr, vvp_code_t code);
extern bool of_FORCE(vthread_t thr, vvp_code_t code);
extern bool of_FORK(vthread_t thr, vvp_code_t code);
extern bool of_INV(vthread_t thr, vvp_code_t code);
extern bool of_IX_ADD(vthread_t thr, vvp_code_t code);
extern bool of_IX_GET(vthread_t thr, vvp_code_t code);
extern bool of_IX_LOAD(vthread_t thr, vvp_code_t code);
extern bool of_IX_MUL(vthread_t thr, vvp_code_t code);
extern bool of_IX_SUB(vthread_t thr, vvp_code_t code);
extern bool of_JMP(vthread_t thr, vvp_code_t code);
extern bool of_JMP0(vthread_t thr, vvp_code_t code);
extern bool of_JMP0XZ(vthread_t thr, vvp_code_t code);
extern bool of_JMP1(vthread_t thr, vvp_code_t code);
extern bool of_JOIN(vthread_t thr, vvp_code_t code);
extern bool of_LOAD(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_MEM(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_NX(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_VEC(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_WR(vthread_t thr, vvp_code_t code);
extern bool of_LOAD_X(vthread_t thr, vvp_code_t code);
extern bool of_LOADI_WR(vthread_t thr, vvp_code_t code);
extern bool of_MOD(vthread_t thr, vvp_code_t code);
extern bool of_MOV(vthread_t thr, vvp_code_t code);
extern bool of_MUL(vthread_t thr, vvp_code_t code);
extern bool of_MUL_WR(vthread_t thr, vvp_code_t code);
extern bool of_MULI(vthread_t thr, vvp_code_t code);
extern bool of_NAND(vthread_t thr, vvp_code_t code);
extern bool of_NANDR(vthread_t thr, vvp_code_t code);
extern bool of_NOOP(vthread_t thr, vvp_code_t code);
extern bool of_NOR(vthread_t thr, vvp_code_t code);
extern bool of_NORR(vthread_t thr, vvp_code_t code);
extern bool of_OR(vthread_t thr, vvp_code_t code);
extern bool of_ORR(vthread_t thr, vvp_code_t code);
extern bool of_RELEASE(vthread_t thr, vvp_code_t code);
extern bool of_SET(vthread_t thr, vvp_code_t code);
extern bool of_SET_MEM(vthread_t thr, vvp_code_t code);
extern bool of_SET_VEC(vthread_t thr, vvp_code_t code);
extern bool of_SET_WORDR(vthread_t thr, vvp_code_t code);
extern bool of_SET_X0(vthread_t thr, vvp_code_t code);
extern bool of_SET_X0_X(vthread_t thr, vvp_code_t code);
extern bool of_SHIFTL_I0(vthread_t thr, vvp_code_t code);
extern bool of_SHIFTR_I0(vthread_t thr, vvp_code_t code);
extern bool of_SHIFTR_S_I0(vthread_t thr, vvp_code_t code);
extern bool of_SUB(vthread_t thr, vvp_code_t code);
extern bool of_SUB_WR(vthread_t thr, vvp_code_t code);
extern bool of_SUBI(vthread_t thr, vvp_code_t code);
extern bool of_VPI_CALL(vthread_t thr, vvp_code_t code);
extern bool of_WAIT(vthread_t thr, vvp_code_t code);
extern bool of_XNOR(vthread_t thr, vvp_code_t code);
extern bool of_XNORR(vthread_t thr, vvp_code_t code);
extern bool of_XOR(vthread_t thr, vvp_code_t code);
extern bool of_XORR(vthread_t thr, vvp_code_t code);

extern bool of_ZOMBIE(vthread_t thr, vvp_code_t code);

extern bool of_FORK_UFUNC(vthread_t thr, vvp_code_t code);
extern bool of_JOIN_UFUNC(vthread_t thr, vvp_code_t code);

extern bool of_CHUNK_LINK(vthread_t thr, vvp_code_t code);

/*
 * This is the format of a machine code instruction.
 */
struct vvp_code_s {
      vvp_code_fun opcode;

      union {
	    unsigned long number;
	    vvp_ipoint_t iptr;
	    vvp_code_t   cptr;
	    vvp_memory_t mem;
	    struct __vpiHandle*handle;
	    struct __vpiScope*scope;
	    functor_t fun_ptr;
      };

      union {
	    unsigned bit_idx[2];
	    vvp_ipoint_t iptr2;
	    vvp_code_t   cptr2;
	    struct ufunc_core*ufunc_core_ptr;
      };
};

/*
 * This function clears the code space, ready for initialization. This
 * needs to be done exactly once before any instructions are created.
 */
extern void codespace_init(void);


/*
 * This function returns a pointer to the next free instruction in the
 * code address space. The codespace_next returns a pointer to the
 * next opcode that will be allocated. This is used by label
 * statements to get the address that will be attached to a label in
 * the code.
 */
extern vvp_code_t codespace_allocate(void);
extern vvp_code_t codespace_next(void);
extern vvp_code_t codespace_null(void);


/*
 * $Log: codes.h,v $
 * Revision 1.65  2004/05/19 03:26:24  steve
 *  Support delayed/non-blocking assignment to reals and others.
 *
 * Revision 1.64  2003/07/03 20:03:36  steve
 *  Remove the vvp_cpoint_t indirect code pointer.
 *
 * Revision 1.63  2003/06/18 03:55:19  steve
 *  Add arithmetic shift operators.
 *
 * Revision 1.62  2003/06/17 19:17:42  steve
 *  Remove short int restrictions from vvp opcodes.
 *
 * Revision 1.61  2003/05/26 04:44:54  steve
 *  Add the set/x0/x instruction.
 *
 * Revision 1.60  2003/05/07 03:39:12  steve
 *  ufunc calls to functions can have scheduling complexities.
 *
 * Revision 1.59  2003/03/28 02:33:56  steve
 *  Add support for division of real operands.
 *
 * Revision 1.58  2003/02/27 20:36:29  steve
 *  Add the cvt/vr instruction.
 *
 * Revision 1.57  2003/02/06 17:41:47  steve
 *  Add the %sub/wr instruction.
 *
 * Revision 1.56  2003/01/26 18:16:22  steve
 *  Add %cvt/ir and %cvt/ri instructions, and support
 *  real values passed as arguments to VPI tasks.
 *
 * Revision 1.55  2003/01/25 23:48:06  steve
 *  Add thread word array, and add the instructions,
 *  %add/wr, %cmp/wr, %load/wr, %mul/wr and %set/wr.
 *
 * Revision 1.54  2002/11/21 22:43:13  steve
 *  %set/x0 instruction to support bounds checking.
 *
 * Revision 1.53  2002/11/08 04:59:57  steve
 *  Add the %assign/v0 instruction.
 *
 * Revision 1.52  2002/11/07 02:32:39  steve
 *  Add vector set and load instructions.
 *
 * Revision 1.51  2002/09/18 04:29:55  steve
 *  Add support for binary NOR operator.
 *
 * Revision 1.50  2002/09/12 15:49:43  steve
 *  Add support for binary nand operator.
 *
 * Revision 1.49  2002/08/28 18:38:07  steve
 *  Add the %subi instruction, and use it where possible.
 *
 * Revision 1.48  2002/08/28 17:15:06  steve
 *  Add the %load/nx opcode to index vpi nets.
 *
 * Revision 1.47  2002/08/22 03:38:40  steve
 *  Fix behavioral eval of x?a:b expressions.
 *
 * Revision 1.46  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.45  2002/07/05 02:50:58  steve
 *  Remove the vpi object symbol table after compile.
 *
 * Revision 1.44  2002/06/02 18:55:58  steve
 *  Add %cmpi/u instruction.
 *
 * Revision 1.43  2002/05/31 20:04:22  steve
 *  Add the %muli instruction.
 *
 * Revision 1.42  2002/05/31 04:09:58  steve
 *  Slight improvement in %mov performance.
 *
 * Revision 1.41  2002/05/29 16:29:34  steve
 *  Add %addi, which is faster to simulate.
 *
 * Revision 1.40  2002/04/21 22:29:49  steve
 *  Add the assign/d instruction for computed delays.
 *
 * Revision 1.39  2002/04/14 18:41:34  steve
 *  Support signed integer division.
 */
#endif
