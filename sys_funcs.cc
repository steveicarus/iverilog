/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: sys_funcs.cc,v 1.1 2004/03/09 04:29:42 steve Exp $"

# include  "config.h"
# include  "compiler.h"

/*
 * Manage the information about system functions. This information is
 * collected from the sources before elaboration and made available
 * via the lookup_sys_func function.
 */

static const struct sfunc_return_type sfunc_table[] = {
      { "$realtime",   NetExpr::ET_REAL,    0, 0 },
      { "$bitstoreal", NetExpr::ET_REAL,    0, 0 },
      { "$itor",       NetExpr::ET_REAL,    0, 0 },
      { "$realtobits", NetExpr::ET_VECTOR, 64, 0 },
      { "$time",       NetExpr::ET_VECTOR, 64, 0 },
      { "$stime",      NetExpr::ET_VECTOR, 32, 0 },
      { "$simtime",    NetExpr::ET_VECTOR, 64, 0 },
      { 0,             NetExpr::ET_VECTOR, 32, 0 }
};


const struct sfunc_return_type* lookup_sys_func(const char*name)
{
      unsigned idx = 0;

      while (sfunc_table[idx].name) {

	    if (strcmp(sfunc_table[idx].name, name) == 0)
		  return sfunc_table + idx;

	    idx += 1;
      }

	/* No luch finding, so return the trailer, which give a
	   default description. */
      return sfunc_table + idx;
}

/*
 * $Log: sys_funcs.cc,v $
 * Revision 1.1  2004/03/09 04:29:42  steve
 *  Separate out the lookup_sys_func table, for eventual
 *  support for function type tables.
 *
 *  Remove ipal compile flags.
 *
 */

