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
#ident "$Id: d-generic-edif.c,v 1.1 2001/09/02 21:33:07 steve Exp $"

# include  "device.h"
# include  "fpga_priv.h"
# include  <assert.h>

static void show_root_ports_edif(ivl_scope_t root)
{
      unsigned cnt = ivl_scope_sigs(root);
      unsigned idx;

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);
	    const char*use_name;

	    const char*dir = 0;
	    switch (ivl_signal_port(sig)) {
		case IVL_SIP_NONE:
		  continue;

		case IVL_SIP_INPUT:
		  dir = "INPUT";
		  break;

		case IVL_SIP_OUTPUT:
		  dir = "OUTPUT";
		  break;

		case IVL_SIP_INOUT:
		  dir = "INOUT";
		  break;
	    }

	    use_name = ivl_signal_basename(sig);
	    if (ivl_signal_pins(sig) == 1) {
		  fprintf(xnf, "            (port %s (direction %s))\n",
			  use_name, dir);

	    } else {
		  unsigned pin;

		  for (pin = 0 ; pin < ivl_signal_pins(sig); pin += 1) {
			fprintf(xnf, "            (port (rename %s_%u "
				"\"%s[%u]\") (direction %s))\n", use_name,
				pin, use_name, pin, dir);
		  }
	    }
      }
}

static void edif_show_header(ivl_design_t des)
{
      ivl_scope_t root = ivl_design_root(des);

	/* write the primitive header */
      fprintf(xnf, "(edif %s\n", ivl_scope_name(root));
      fprintf(xnf, "    (edifVersion 2 0 0)\n");
      fprintf(xnf, "    (edifLevel 0)\n");
      fprintf(xnf, "    (keywordMap (keywordLevel 0))\n");
      fprintf(xnf, "    (status\n");
      fprintf(xnf, "     (written\n");
      fprintf(xnf, "        (timeStamp 0 0 0 0 0 0)\n");
      fprintf(xnf, "        (author \"unknown\")\n");
      fprintf(xnf, "        (program \"Icarus Verilog/fpga.tgt\")))\n");

	/* Write out the external references here? */

	/* Write out the library header */
      fprintf(xnf, "    (library DESIGN\n");
      fprintf(xnf, "      (edifLevel 0)\n");
      fprintf(xnf, "      (technology (numberDefinition))\n");

	/* The root module is a cell in the library. */
      fprintf(xnf, "      (cell %s\n", ivl_scope_name(root));
      fprintf(xnf, "        (cellType GENERIC)\n");
      fprintf(xnf, "        (view Netlist_representation\n");
      fprintf(xnf, "          (viewType NETLIST)\n");
      fprintf(xnf, "          (interface\n");

      show_root_ports_edif(root);

      fprintf(xnf, "          )\n"); /* end the (interface ) sexp */

      fprintf(xnf, "          (contents\n");
}

static void edif_show_footer(ivl_design_t des)
{
      ivl_scope_t root = ivl_design_root(des);

      fprintf(xnf, "          )\n"); /* end the (contents ) sexp */
      fprintf(xnf, "        )\n"); /* end the (view ) sexp */
      fprintf(xnf, "      )\n"); /* end the (cell ) sexp */
      fprintf(xnf, "    )\n"); /* end the (library ) sexp */

	/* Make an instance of the defined object */
      fprintf(xnf, "    (design %s\n", ivl_scope_name(root));
      fprintf(xnf, "      (cellRef %s (libraryRef DESIGN))\n",
	      ivl_scope_name(root));

      if (part)
	    fprintf(xnf, "      (property PART (string \"%s\"))\n", part);

      fprintf(xnf, "    )\n");

      fprintf(xnf, ")\n"); /* end the (edif  ) sexp */
}

static void edif_show_logic(ivl_net_logic_t net)
{
      switch (ivl_logic_type(net)) {

	  case IVL_LO_BUF:
	    assert(ivl_logic_pins(net) == 2);
	    fprintf(xnf, "           (instance");
	    fprintf(xnf, " %s", ivl_logic_name(net));
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef BUF (libraryRef VIRTEX))))\n");
	    break;

      }
}

static void edif_show_dff(ivl_lpm_t net)
{
}


const struct device_s d_generic_edif = {
      edif_show_header,
      edif_show_footer,
      edif_show_logic,
      edif_show_dff,
      0,
      0,
      0,
      0
};


/*
 * $Log: d-generic-edif.c,v $
 * Revision 1.1  2001/09/02 21:33:07  steve
 *  Rearrange the XNF code generator to be generic-xnf
 *  so that non-XNF code generation is also possible.
 *
 *  Start into the virtex EDIF output driver.
 *
 */

