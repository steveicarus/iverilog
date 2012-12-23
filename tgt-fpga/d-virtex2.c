/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "device.h"
# include  "fpga_priv.h"
# include  "edif.h"
# include  "generic.h"
# include  "xilinx.h"
# include  <stdlib.h>
# include  <string.h>
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
