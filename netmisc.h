#ifndef __netmisc_H
#define __netmisc_H
/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: netmisc.h,v 1.7 2000/05/14 17:55:04 steve Exp $"
#endif

# include  "netlist.h"

/*
 * This funciton transforms an expression by padding the high bits
 * with V0 until the expression has the desired width. This may mean
 * not transforming the expression at all, if it is already wide
 * enough.
 */
extern NetExpr*pad_to_width(NetExpr*expr, unsigned wid);
extern NetNet*pad_to_width(Design*des, const string&p, NetNet*n, unsigned w);

/*
 * This function chooses a nexus name for the link. The algorithm is
 * such that any signal in the link will have the same nexus name, and
 * signals that are not connected together will have a different nexus
 * name.
 */
extern string nexus_from_link(const Link*lnk);

/*
 * Check to see if the link has a constant value driven to it. If
 * there is only a NetConst driving this pin, the return a pointer to
 * that NetConst object. Also, return the index of the bit in that
 * constant through the idx parameter.
 */
extern NetConst* link_const_value(Link&pin, unsigned&idx);

/*
 * This local function returns true if all the the possible drivers of
 * this link are constant. It will also return true if there are no
 * drivers at all.
 */
extern bool link_drivers_constant(const Link&lnk);

/*
 * This function returns the value of the constant driving this link,
 * or Vz if there is no constant. The results of this function are
 * only meaningful if link_drivers_constant(lnk) == true.
 */
extern verinum::V driven_value(const Link&lnk);

/*
 * $Log: netmisc.h,v $
 * Revision 1.7  2000/05/14 17:55:04  steve
 *  Support initialization of FF Q value.
 *
 * Revision 1.6  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.5  2000/04/20 00:28:03  steve
 *  Catch some simple identity compareoptimizations.
 *
 * Revision 1.4  2000/03/16 19:03:03  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 * Revision 1.3  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  2000/02/16 03:58:27  steve
 *  Fix up width matching in structural bitwise operators.
 *
 * Revision 1.1  1999/09/29 00:42:51  steve
 *  Allow expanding of additive operators.
 *
 */
#endif
