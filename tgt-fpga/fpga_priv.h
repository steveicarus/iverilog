#ifndef IVL_fpga_priv_H
#define IVL_fpga_priv_H
/*
 * Copyright (c) 2001-2014 Stephen Williams (steve@icarus.com)
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

# include  <stdio.h>
# include  "device.h"

/* This is the opened xnf file descriptor. It is the output that this
   code generator writes to, whether the format is XNF or EDIF. */
extern FILE*xnf;

extern int show_scope_gates(ivl_scope_t net, void*x);


extern device_t device;

extern const char*part;
extern const char*arch;

/*
 * Attribute lookup, should this be provided in ivl_target.h?
 */
int scope_has_attribute(ivl_scope_t s, const char *name);

/*
 * These are mangle functions.
 */
extern void xnf_mangle_logic_name(ivl_net_logic_t net, char*buf, size_t nbuf);
extern void xnf_mangle_lpm_name(ivl_lpm_t net, char*buf, size_t nbuf);

extern const char*xnf_mangle_nexus_name(ivl_nexus_t net);

#endif /* IVL_fpga_priv_H */
