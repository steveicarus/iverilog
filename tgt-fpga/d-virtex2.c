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
#ident "$Id: d-virtex2.c,v 1.20 2004/10/04 01:10:57 steve Exp $"
#endif

# include  "device.h"
# include  "fpga_priv.h"
# include  "edif.h"
# include  "generic.h"
# include  "xilinx.h"
# include  <stdlib.h>
# include  <string.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>


/*
 * This is a table of cell types that are accessible via the cellref
 * attribute to a gate.
 */
const static struct edif_xlib_celltable virtex2_celltable[] = {
      { "BUFG",     xilinx_cell_bufg },
      { "MULT_AND", xilinx_cell_mult_and },
      { 0, 0}
};

/*
 * The show_header function is called before any of the devices of the
 * netlist are scanned.
 *
 * In this function, we look at the ports of the root module to decide
 * if they are to be made into ports. Modules that have PAD attributes
 * are *not* to be used as ports, they will be connected to special
 * PAD devices instead.
 */
static void virtex2_show_header(ivl_design_t des)
{
      const char*part_str = 0;

      xilinx_common_header(des);

      xlib = edif_xlibrary_create(edf, "VIRTEX2");
      edif_xlibrary_set_celltable(xlib, virtex2_celltable);


      if ( (part_str = ivl_design_flag(des, "part")) && (part_str[0] != 0) ) {
	    edif_pstring(edf, "PART", part_str);
      }

      cell_0 = edif_xcell_create(xlib, "GND", 1);
      edif_cell_portconfig(cell_0, 0, "GROUND", IVL_SIP_OUTPUT);

      cell_1 = edif_xcell_create(xlib, "VCC", 1);
      edif_cell_portconfig(cell_1, 0, "VCC", IVL_SIP_OUTPUT);

}

const struct device_s d_virtex2_edif = {
      virtex2_show_header,
      xilinx_show_footer,
      xilinx_show_scope,
      xilinx_pad,
      virtex_logic,
      virtex_generic_dff,
      virtex_eq,
      virtex_eq,
      virtex_ge,
      0, /* show_cmp_gt */
      virtex_mux,
      virtex_add,
      virtex_add,
      xilinx_shiftl, /* show_shiftl */
      0  /* show_shiftr */
};


/*
 * $Log: d-virtex2.c,v $
 * Revision 1.20  2004/10/04 01:10:57  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.19  2003/11/12 03:20:14  steve
 *  devices need show_cmp_gt
 *
 * Revision 1.18  2003/07/04 00:10:09  steve
 *  Generate MUXF5 based 4-input N-wide muxes.
 *
 * Revision 1.17  2003/07/02 03:02:15  steve
 *  More xilinx common code.
 *
 * Revision 1.16  2003/06/28 04:18:47  steve
 *  Add support for wide OR/NOR gates.
 *
 * Revision 1.15  2003/06/26 03:57:05  steve
 *  Add Xilinx support for A/B MUX devices.
 *
 * Revision 1.14  2003/06/25 02:55:57  steve
 *  Virtex and Virtex2 share much code.
 *
 * Revision 1.13  2003/06/25 01:49:06  steve
 *  Spelling fixes.
 *
 * Revision 1.12  2003/06/25 01:46:44  steve
 *  Virtex support for NOT gates.
 *
 * Revision 1.11  2003/06/24 03:55:00  steve
 *  Add ivl_synthesis_cell support for virtex2.
 */

