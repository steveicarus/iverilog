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
#ident "$Id: codes.h,v 1.14 2001/04/01 07:22:08 steve Exp $"
#endif


# include  "pointers.h"
# include  "vthread.h"
# include  <stdio.h>

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
extern bool of_CMPU(vthread_t thr, vvp_code_t code);
extern bool of_CMPX(vthread_t thr, vvp_code_t code);
extern bool of_CMPZ(vthread_t thr, vvp_code_t code);
extern bool of_DELAY(vthread_t thr, vvp_code_t code);
extern bool of_END(vthread_t thr, vvp_code_t code);
extern bool of_FORK(vthread_t thr, vvp_code_t code);
extern bool of_INV(vthread_t thr, vvp_code_t code);
extern bool of_JMP(vthread_t thr, vvp_code_t code);
extern bool of_JMP0(vthread_t thr, vvp_code_t code);
extern bool of_JMP0XZ(vthread_t thr, vvp_code_t code);
extern bool of_JMP1(vthread_t thr, vvp_code_t code);
extern bool of_JOIN(vthread_t thr, vvp_code_t code);
extern bool of_LOAD(vthread_t thr, vvp_code_t code);
extern bool of_MOV(vthread_t thr, vvp_code_t code);
extern bool of_NOOP(vthread_t thr, vvp_code_t code);
extern bool of_OR(vthread_t thr, vvp_code_t code);
extern bool of_SET(vthread_t thr, vvp_code_t code);
extern bool of_VPI_CALL(vthread_t thr, vvp_code_t code);
extern bool of_WAIT(vthread_t thr, vvp_code_t code);

/*
 * This is the format of a machine code instruction.
 */
struct vvp_code_s {
      vvp_code_fun opcode;

      union {
	    unsigned number;
	    vvp_ipoint_t iptr;
	    vvp_cpoint_t cptr;
	    struct __vpiHandle*handle;
      };

      unsigned short bit_idx1;
      unsigned short bit_idx2;
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

extern void codespace_dump(FILE*fd);

/*
 * $Log: codes.h,v $
 * Revision 1.14  2001/04/01 07:22:08  steve
 *  Implement the less-then and %or instructions.
 *
 * Revision 1.13  2001/04/01 06:40:44  steve
 *  Support empty statements for hanging labels.
 *
 * Revision 1.12  2001/04/01 06:12:13  steve
 *  Add the bitwise %and instruction.
 *
 * Revision 1.11  2001/04/01 04:34:28  steve
 *  Implement %cmp/x and %cmp/z instructions.
 *
 * Revision 1.10  2001/03/31 17:36:02  steve
 *  Add the jmp/1 instruction.
 *
 * Revision 1.9  2001/03/31 01:59:58  steve
 *  Add the ADD instrunction.
 *
 * Revision 1.8  2001/03/30 04:55:22  steve
 *  Add fork and join instructions.
 *
 * Revision 1.7  2001/03/26 04:00:39  steve
 *  Add the .event statement and the %wait instruction.
 *
 * Revision 1.6  2001/03/25 03:54:26  steve
 *  Add JMP0XZ and postpone net inputs when needed.
 *
 * Revision 1.5  2001/03/22 05:08:00  steve
 *  implement %load, %inv, %jum/0 and %cmp/u
 *
 * Revision 1.4  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.3  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 * Revision 1.2  2001/03/11 23:06:49  steve
 *  Compact the vvp_code_s structure.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
