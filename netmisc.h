#ifndef __netmisc_H
#define __netmisc_H
/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"

/*
 * Search for a symbol using the "start" scope as the starting
 * point. If the path includes a scope part, then locate the
 * scope first.
 *
 * The return value is the scope where the symbol was found.
 * If the symbol was not found, return 0. The output arguments
 * get 0 except for the pointer to the object that represents
 * the located symbol.
 */
extern NetScope* symbol_search(Design*des, NetScope*start, hname_t path,
			       NetNet*&net,       /* net/reg */
			       NetMemory*&mem,    /* memory */
			       NetVariable*&var,  /* real/realtime */
			       const NetExpr*&par,/* parameter */
			       NetEvent*&eve       /* named event */);

/*
 * This function transforms an expression by padding the high bits
 * with V0 until the expression has the desired width. This may mean
 * not transforming the expression at all, if it is already wide
 * enough.
 */
extern NetExpr*pad_to_width(NetExpr*expr, unsigned wid);
extern NetNet*pad_to_width(Design*des, NetNet*n, unsigned w);

/*
 * This function takes as input a NetNet signal and adds a constant
 * value to it. If the val is 0, then simply return sig. Otherwise,
 * return a new NetNet value that is the output of an addition.
 */
extern NetNet*add_to_net(Design*des, NetNet*sig, long val);

/*
 * Calculate the reduction OR from the input signal.
 */
extern NetNet*reduction_or(Design*des, NetNet*sig);

/*
 * In some cases the lval is accessible as a pointer to the head of
 * a list of NetAssign_ objects. This function returns the width of
 * the l-value represented by this list.
 */
extern unsigned count_lval_width(const class NetAssign_*first);

/*
 * This function elaborates an expression, and tries to evaluate it
 * right away. If the expression can be evaluated, this returns a
 * constant expression. If it cannot be evaluated, it returns whatever
 * it can. If the expression cannot be elaborated, return 0.
 */
class PExpr;
extern NetExpr* elab_and_eval(Design*des, NetScope*scope, const PExpr*pe);

#endif
