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
#ident "$Id: codes.h,v 1.2 2001/03/11 23:06:49 steve Exp $"
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
extern bool of_ASSIGN(vthread_t thr, vvp_code_t code);
extern bool of_DELAY(vthread_t thr, vvp_code_t code);
extern bool of_END(vthread_t thr, vvp_code_t code);
extern bool of_SET(vthread_t thr, vvp_code_t code);
extern bool of_NOOP(vthread_t thr, vvp_code_t code);

/*
 * This is the format of a machine code instruction.
 */
struct vvp_code_s {
      vvp_code_fun opcode;

      union {
	    unsigned number;
	    vvp_ipoint_t iptr;
	    vvp_cpoint_t cptr;
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
 * Revision 1.2  2001/03/11 23:06:49  steve
 *  Compact the vvp_code_s structure.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
