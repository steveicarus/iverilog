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
#ident "$Id: d-virtex.c,v 1.1 2001/09/06 04:28:40 steve Exp $"

# include  "device.h"
# include  "fpga_priv.h"
# include  <stdlib.h>
# include  <string.h>
# include  <malloc.h>
# include  <assert.h>

/*
 * This is the EDIF code generator for VIRTEX style parts. It uses the
 * VIRTEX primitives from the unified library.
 */

static const char*virtex_library_text =
"    (external VIRTEX (edifLevel 0) (technology (numberDefinition))\n"
"      (cell AND2 (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT)))))\n"
"      (cell BUF (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I (direction INPUT)))))\n"
"      (cell FDCE (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port Q (direction OUTPUT))\n"
"                 (port D (direction INPUT))\n"
"                 (port C (direction INPUT))\n"
"                 (port CE (direction INPUT)))))\n"
"      (cell GND (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface (port G (direction OUTPUT)))))\n"
"      (cell NOR2 (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT)))))\n"
"      (cell NOR3 (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT))\n"
"                 (port I2 (direction INPUT)))))\n"
"      (cell VCC (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface (port P (direction OUTPUT)))))\n"
"    )\n"
;


static void edif_show_header(ivl_design_t des)
{
      edif_show_header_generic(des, virtex_library_text);
}

static void edif_show_logic(ivl_net_logic_t net)
{
      char jbuf[1024];
      unsigned idx;

      edif_uref += 1;

      switch (ivl_logic_type(net)) {

	  case IVL_LO_AND:
	    assert(ivl_logic_pins(net) <= 10);
	    assert(ivl_logic_pins(net) >= 3);

	    fprintf(xnf, "(instance (rename U%u \"%s\")",
		    edif_uref, ivl_logic_name(net));
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef AND%u (libraryRef VIRTEX))))\n",
		    ivl_logic_pins(net) - 1);

	    sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 0), jbuf);

	    for (idx = 1 ;  idx < ivl_logic_pins(net)  ;  idx += 1) {
		  sprintf(jbuf, "(portRef I%u (instanceRef U%u))",
			  idx-1, edif_uref);
		  edif_set_nexus_joint(ivl_logic_pin(net, idx), jbuf);
	    }
	    break;

	  case IVL_LO_BUF:
	    assert(ivl_logic_pins(net) == 2);
	    fprintf(xnf, "(instance (rename U%u \"%s\")",
		    edif_uref, ivl_logic_name(net));
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef BUF (libraryRef VIRTEX))))\n");

	    sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 0), jbuf);

	    sprintf(jbuf, "(portRef I (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 1), jbuf);
	    break;

	  case IVL_LO_NOR:
	    assert(ivl_logic_pins(net) <= 10);
	    assert(ivl_logic_pins(net) >= 3);

	    fprintf(xnf, "(instance (rename U%u \"%s\")",
		    edif_uref, ivl_logic_name(net));
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef NOR%u (libraryRef VIRTEX))))\n",
		    ivl_logic_pins(net) - 1);

	    sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 0), jbuf);

	    for (idx = 1 ;  idx < ivl_logic_pins(net)  ;  idx += 1) {
		  sprintf(jbuf, "(portRef I%u (instanceRef U%u))",
			  idx-1, edif_uref);
		  edif_set_nexus_joint(ivl_logic_pin(net, idx), jbuf);
	    }
	    break;

	  default:
	    fprintf(stderr, "UNSUPPORT LOGIC TYPE: %u\n", ivl_logic_type(net));
      }
}


const struct device_s d_virtex_edif = {
      edif_show_header,
      edif_show_footer,
      edif_show_logic,
      edif_show_generic_dff,
      0,
      0,
      0,
      0
};


/*
 * $Log: d-virtex.c,v $
 * Revision 1.1  2001/09/06 04:28:40  steve
 *  Separate the virtex and generic-edif code generators.
 *
 */

