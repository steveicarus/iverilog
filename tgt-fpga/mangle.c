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
#ident "$Id: mangle.c,v 1.1 2001/08/28 04:14:20 steve Exp $"


# include  "fpga_priv.h"
# include  <string.h>

static size_t mangle_scope_name(ivl_scope_t net, char*buf, size_t nbuf)
{
      unsigned cnt = 0;
      ivl_scope_t parent = ivl_scope_parent(net);

      if (parent) {
	    cnt = mangle_scope_name(parent, buf, nbuf);
	    buf += cnt;
	    nbuf -= cnt;
	    *buf++ = '/';
	    nbuf -= 1;
	    cnt += 1;
      }

      strcpy(buf, ivl_scope_basename(net));
      cnt += strlen(buf);

      return cnt;
}

void mangle_logic_name(ivl_net_logic_t net, char*buf, size_t nbuf)
{
      size_t cnt = mangle_scope_name(ivl_logic_scope(net), buf, nbuf);
      buf[cnt++] = '/';
      strcpy(buf+cnt, ivl_logic_basename(net));
}

void mangle_lpm_name(ivl_lpm_t net, char*buf, size_t nbuf)
{
      size_t cnt = mangle_scope_name(ivl_lpm_scope(net), buf, nbuf);
      buf[cnt++] = '/';
      strcpy(buf+cnt, ivl_lpm_basename(net));
}


/*
 * $Log: mangle.c,v $
 * Revision 1.1  2001/08/28 04:14:20  steve
 *  Add the fpga target.
 *
 */

