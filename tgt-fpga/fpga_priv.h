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
#ident "$Id: fpga_priv.h,v 1.6 2002/08/12 01:35:03 steve Exp $"
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
 * These are mangle functions.
 */
extern void xnf_mangle_logic_name(ivl_net_logic_t net, char*buf, size_t nbuf);
extern void xnf_mangle_lpm_name(ivl_lpm_t net, char*buf, size_t nbuf);

extern const char*xnf_mangle_nexus_name(ivl_nexus_t net);

/*
 * These are generic EDIF functions that EDIF targets use.
 *
 * edif_show_header_generic
 *   This function draws the header part of the EDIF file, including
 *   the ports of the module, if there are any. Also include the
 *   library string where the external library would go.
 *
 * edif_show_footer
 *   This completes the net items, draws the constant references, then
 *   writes out the final declarations of the EDIF file.
 *
 * edif_set_nexus_joint
 *   This stores joint information in the nexus, and save the nexus in
 *   a list the edif_show_footer function later uses that list to draw
 *   all the join records.
 *
 * edif_show_generic_dff
 *   The edif DFF is an FDCE. This function draws an FDCE for the lpm
 *   DFF of the design.
 *
 * edif_uref
 *   This global variable keeps count of the devices drawn. Since the
 *   EDIF format has very simple names, each device instead has a uref
 *   and a name of the form U%u. A (rename U% "foo") preserves the
 *   real name.
 */
extern void edif_show_header_generic(ivl_design_t des, const char*library);
extern void edif_show_footer(ivl_design_t des);
extern void edif_set_nexus_joint(ivl_nexus_t nex, const char*joint);

extern void edif_show_generic_dff(ivl_lpm_t net);

extern unsigned edif_uref;


/*
 * $Log: fpga_priv.h,v $
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
