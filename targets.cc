/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: targets.cc,v 1.1 1998/11/03 23:29:07 steve Exp $"
#endif

# include "target.h"

extern const struct target tgt_verilog;
extern const struct target tgt_vvm;
const struct target *target_table[] = {
      &tgt_verilog,
      &tgt_vvm,
      0
};

/*
 * $Log: targets.cc,v $
 * Revision 1.1  1998/11/03 23:29:07  steve
 *  Introduce verilog to CVS.
 *
 */

