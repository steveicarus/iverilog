#ifndef __codes_H
#define __codes_H
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
#ident "$Id: codes.h,v 1.26 2001/05/09 04:23:18 steve Exp $"
#endif


# include  "pointers.h"
# include  "memory.h"
# include  "vthread.h"

typedef struct vvp_code_s *vvp_code_t;
typedef bool (*vvp_code_fun)(vthread_t thr, vvp_code_t code);

/*
 * These functions are implementations of executable op-codes. The
 * implementation lives in the vthread.cc file so that they have
 * access to the thread context.
 */
extern bool of_ADD(vthread_t thr, vvp_code_t code);
extern bool of_AND(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN(vthread_t thr, vvp_code_t code);
extern bool of_ASSIGN_MEM(vthread_t thr, vvp_code_t code);
extern bool of_BREAKPOINT(vthread_t thr, vvp_code_t code);
extern bool of_CMPS(vthread_t thr, vvp_code_t code);
extern bool of_CMPU(vthread_t thr, vvp_code_t code);
extern bool of_CMPX(vthread_t thr, vvp_code_t code);
extern bool of_CMPZ(vthread_t thr, vvp_code_t code);
extern bool of_DELAY(vthread_t thr, vvp_code_t code);
extern bool of_DISABLE(vthread_t thr, vvp_code_t code);
extern bool of_END(vthread_t thr, vvp_code_t code);
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
extern bool of_MOV(vthread_t thr, vvp_code_t code);
extern bool of_NOOP(vthread_t thr, vvp_code_t code);
extern bool of_NORR(vthread_t thr, vvp_code_t code);
extern bool of_OR(vthread_t thr, vvp_code_t code);
extern bool of_SET(vthread_t thr, vvp_code_t code);
extern bool of_SET_MEM(vthread_t thr, vvp_code_t code);
extern bool of_SUB(vthread_t thr, vvp_code_t code);
extern bool of_VPI_CALL(vthread_t thr, vvp_code_t code);
extern bool of_WAIT(vthread_t thr, vvp_code_t code);
extern bool of_XNOR(vthread_t thr, vvp_code_t code);
extern bool of_XOR(vthread_t thr, vvp_code_t code);

extern bool of_ZOMBIE(vthread_t thr, vvp_code_t code);

/*
 * This is the format of a machine code instruction.
 */
struct vvp_code_s {
      vvp_code_fun opcode;

      union {
	    unsigned number;
	    vvp_ipoint_t iptr;
	    vvp_cpoint_t cptr;
	    vvp_memory_t mem;
	    struct __vpiHandle*handle;
	    struct fork_extend*fork;
      };

      unsigned short bit_idx1;
      unsigned short bit_idx2;
};

struct fork_extend {
      vvp_cpoint_t cptr;
      struct __vpiScope*scope;
};


/*
 * This function clears the code space, ready for initialization. This
 * needs to be done exactly once before any instructions are created.
 */
extern void codespace_init(void);

/*
 * This function returns a pointer to the next free instruction in the
 * code address space.
 */
extern vvp_cpoint_t codespace_allocate(void);

extern vvp_cpoint_t codespace_next(void);


/*
 * Return a pointer to the indexed instruction in the codespace. The
 * ptr must be a value returned from codespace_allocate. The compiler
 * can use this to get a handle on the instruction to be created, and
 * the runtime uses this to get the instruction addressed by the PC or
 * by a branch instruction.
 */
extern vvp_code_t codespace_index(vvp_cpoint_t ptr);


/*
 * $Log: codes.h,v $
 * Revision 1.26  2001/05/09 04:23:18  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.25  2001/05/06 17:42:22  steve
 *  Add the %ix/get instruction. (Stephan Boettcher)
 *
 * Revision 1.24  2001/05/05 23:55:46  steve
 *  Add the beginnings of an interactive debugger.
 *
 * Revision 1.23  2001/05/02 23:16:50  steve
 *  Document memory related opcodes,
 *  parser uses numbv_s structures instead of the
 *  symbv_s and a mess of unions,
 *  Add the %is/sub instruction.
 *        (Stephan Boettcher)
 *
 * Revision 1.22  2001/05/02 01:57:25  steve
 *  Support behavioral subtraction.
 *
 * Revision 1.21  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
 *
 * Revision 1.20  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.19  2001/04/15 16:37:48  steve
 *  add XOR support.
 *
 * Revision 1.18  2001/04/15 04:07:56  steve
 *  Add support for behavioral xnor.
 *
 * Revision 1.17  2001/04/13 03:55:18  steve
 *  More complete reap of all threads.
 *
 * Revision 1.16  2001/04/05 01:12:28  steve
 *  Get signed compares working correctly in vvp.
 *
 * Revision 1.15  2001/04/01 22:25:33  steve
 *  Add the reduction nor instruction.
 */
#endif
