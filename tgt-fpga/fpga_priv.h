#ifndef __fpga_priv_H
#define __fpga_priv_H
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: fpga_priv.h,v 1.8 2003/07/02 00:48:03 steve Exp $"
#endif

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


/*
 * $Log: fpga_priv.h,v $
 * Revision 1.8  2003/07/02 00:48:03  steve
 *  No longer export generic-edif functions.
 *
 * Revision 1.7  2003/06/24 03:55:00  steve
 *  Add ivl_synthesis_cell support for virtex2.
 *
 * Revision 1.6  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.5  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 * Revision 1.4  2001/09/06 04:28:40  steve
 *  Separate the virtex and generic-edif code generators.
 *
 * Revision 1.3  2001/09/02 21:33:07  steve
 *  Rearrange the XNF code generator to be generic-xnf
 *  so that non-XNF code generation is also possible.
 *
 *  Start into the virtex EDIF output driver.
 *
 * Revision 1.2  2001/08/30 04:31:04  steve
 *  Mangle nexus names.
 *
 * Revision 1.1  2001/08/28 04:14:20  steve
 *  Add the fpga target.
 *
 */
#endif
