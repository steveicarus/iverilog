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
#ident "$Id: sys_priv.h,v 1.1 2002/08/15 02:12:20 steve Exp $"
#endif

/*
 * This function is used by sys_vcd and sys_lxt as the dumpvars
 * compiletf function.
 */
extern int sys_vcd_dumpvars_compiletf(char*name);

/*
 * $Log: sys_priv.h,v $
 * Revision 1.1  2002/08/15 02:12:20  steve
 *  add dumpvars_compiletf to check first argument.
 *
 */
#endif
