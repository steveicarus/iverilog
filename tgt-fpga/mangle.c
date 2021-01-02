/*
 * Copyright (c) 2001-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "fpga_priv.h"
# include  <string.h>
# include  <stdlib.h>
# include  "ivl_alloc.h"

static size_t xnf_mangle_scope_name(ivl_scope_t net, char*buf, size_t nbuf)
{
      unsigned cnt = 0;
      ivl_scope_t parent = ivl_scope_parent(net);

      if (parent) {
	    cnt = xnf_mangle_scope_name(parent, buf, nbuf);
	    buf += cnt;
	    *buf++ = '/';
	    cnt += 1;
      }

      strcpy(buf, ivl_scope_basename(net));
      cnt += strlen(buf);

      return cnt;
}

void xnf_mangle_logic_name(ivl_net_logic_t net, char*buf, size_t nbuf)
{
      size_t cnt = xnf_mangle_scope_name(ivl_logic_scope(net), buf, nbuf);
      buf[cnt++] = '/';
      strcpy(buf+cnt, ivl_logic_basename(net));
}

void xnf_mangle_lpm_name(ivl_lpm_t net, char*buf, size_t nbuf)
{
      size_t cnt = xnf_mangle_scope_name(ivl_lpm_scope(net), buf, nbuf);
      buf[cnt++] = '/';
      strcpy(buf+cnt, ivl_lpm_basename(net));
}

/*
 * Nexus names are used in pin records to connect things together. It
 * almost doesn't matter what the nexus name is, but for readability
 * we choose a name that is close to the nexus name. This function
 * converts the existing name to a name that XNF can use.
 *
 * For speed, this function saves the calculated string into the real
 * nexus by using the private pointer. Every nexus is used at least
 * twice, so this cuts the mangling time in half at least.
 */
const char* xnf_mangle_nexus_name(ivl_nexus_t net)
{
      char*name = ivl_nexus_get_private(net);
      char*cp;

      if (name != 0) {
	    return name;
      }

      name = malloc(strlen(ivl_nexus_name(net)) + 1);
      strcpy(name, ivl_nexus_name(net));

      for (cp = name ;  *cp ;  cp += 1) switch (*cp) {

	  case '.':
	    *cp = '/';
	    break;
	  default:
	    break;
      }

      ivl_nexus_set_private(net, name);
      return name;
}
