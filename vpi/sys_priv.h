#ifndef __vpi_sys_priv_H
#define __vpi_sys_priv_H
/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: sys_priv.h,v 1.5 2004/01/21 01:22:53 steve Exp $"
#endif

# include  "vpi_config.h"
# include  "vpi_user.h"

/*
 * This function is used by sys_vcd and sys_lxt as the dumpvars
 * compiletf function.
 */
extern int sys_vcd_dumpvars_compiletf(char*name);

/*
 * Context structure for PRNG in mt19937int.c
 */
struct context_s {
      int		mti;		/* the array for the state vector */
      unsigned long	mt[1023];	/* mti==N+1 means mt[N] is not init */
};

extern void sgenrand(struct context_s *context, unsigned long seed);
extern unsigned long genrand(struct context_s *context);


extern int is_constant(vpiHandle obj);

extern PLI_UINT64 timerec_to_time64(const struct t_vpi_time*time);

/*
 * $Log: sys_priv.h,v $
 * Revision 1.5  2004/01/21 01:22:53  steve
 *  Give the vip directory its own configure and vpi_config.h
 *
 * Revision 1.4  2003/10/30 03:43:20  steve
 *  Rearrange fileio functions, and add ungetc.
 *
 * Revision 1.3  2003/09/30 01:33:39  steve
 *  dumpers must be aware of 64bit time.
 *
 * Revision 1.2  2003/05/14 04:18:16  steve
 *  Use seed to store random number context.
 *
 * Revision 1.1  2002/08/15 02:12:20  steve
 *  add dumpvars_compiletf to check first argument.
 *
 */
#endif
