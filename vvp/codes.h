#ifndef __codes_H
#define __codes_H
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
extern bool of_MOD_S(vthread_t thr, vvp_code_t code);
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

#endif
