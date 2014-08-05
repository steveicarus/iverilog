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

# include  "device.h"
# include  "fpga_priv.h"
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  "ivl_alloc.h"

struct nexus_recall {
      struct nexus_recall*next;
      ivl_nexus_t nex;
      char* joined;
};
static struct nexus_recall*net_list = 0;

static unsigned edif_uref = 0;

static void edif_set_nexus_joint(ivl_nexus_t nex, const char*joint)
{
      size_t newlen;
      struct nexus_recall*rec;

      rec = (struct nexus_recall*)ivl_nexus_get_private(nex);
      if (rec == 0) {
	    rec = malloc(sizeof(struct nexus_recall));
	    rec->nex = nex;
	    rec->joined = malloc(8);
	    rec->joined[0] = 0;
	    rec->next = net_list;
	    net_list = rec;
	    ivl_nexus_set_private(nex, rec);
      }

      newlen = strlen(rec->joined) + strlen(joint) + 2;
      rec->joined = realloc(rec->joined, newlen);
      strcat(rec->joined, " ");
      strcat(rec->joined, joint);
}


static void show_root_ports_edif(ivl_scope_t root)
{
      char jbuf[1024];
      unsigned cnt = ivl_scope_sigs(root);
      unsigned idx;

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);
	    const char*use_name;
	    const char*dir = 0;

	    if (ivl_signal_attr(sig, "PAD") != 0)
		  continue;

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

		  sprintf(jbuf, "(portRef %s)", use_name);
		  edif_set_nexus_joint(ivl_signal_pin(sig, 0), jbuf);

	    } else {
		  unsigned pin;

		  for (pin = 0 ; pin < ivl_signal_pins(sig); pin += 1) {
			fprintf(xnf, "            (port (rename %s_%u "
				"\"%s[%u]\") (direction %s))\n", use_name,
				pin, use_name, pin, dir);
			sprintf(jbuf, "(portRef %s_%u)", use_name, pin);
			edif_set_nexus_joint(ivl_signal_pin(sig, pin), jbuf);
		  }
	    }
      }
}


static void edif_show_header_generic(ivl_design_t des, const char*library)
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
      fputs(library, xnf);

	/* Write out the library header */
      fprintf(xnf, "    (library DESIGN\n");
      fprintf(xnf, "      (edifLevel 0)\n");
      fprintf(xnf, "      (technology (numberDefinition))\n");

	/* The root module is a cell in the library. */
      fprintf(xnf, "      (cell %s\n", ivl_scope_name(root));
      fprintf(xnf, "        (cellType GENERIC)\n");
      fprintf(xnf, "        (view net\n");
      fprintf(xnf, "          (viewType NETLIST)\n");
      fprintf(xnf, "          (interface\n");

      show_root_ports_edif(root);

      fprintf(xnf, "          )\n"); /* end the (interface ) sexp */

      fprintf(xnf, "          (contents\n");
}

static const char*external_library_text =
"    (external VIRTEX (edifLevel 0) (technology (numberDefinition))\n"
"      (cell AND2 (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT)))))\n"
"      (cell BUF (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I (direction INPUT)))))\n"
"      (cell FDCE (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port Q (direction OUTPUT))\n"
"                 (port D (direction INPUT))\n"
"                 (port C (direction INPUT))\n"
"                 (port CE (direction INPUT))\n"
"                 (port CLR (direction INPUT)))))\n"
"      (cell FDCPE (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port Q (direction OUTPUT))\n"
"                 (port D (direction INPUT))\n"
"                 (port C (direction INPUT))\n"
"                 (port CE (direction INPUT))\n"
"                 (port PRE (direction INPUT))\n"
"                 (port CLR (direction INPUT)))))\n"
"      (cell GND (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface (port G (direction OUTPUT)))))\n"
"      (cell NOR2 (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT)))))\n"
"      (cell NOR3 (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT))\n"
"                 (port I2 (direction INPUT)))))\n"
"      (cell VCC (cellType GENERIC)\n"
"            (view net\n"
"              (viewType NETLIST)\n"
"              (interface (port P (direction OUTPUT)))))\n"
"    )\n"
;

static void edif_show_header(ivl_design_t des)
{
      edif_show_header_generic(des, external_library_text);
}

static void edif_show_consts(ivl_design_t des)
{
      unsigned idx;
      char jbuf[128];

      for (idx = 0 ;  idx < ivl_design_consts(des) ;  idx += 1) {
	    unsigned pin;
	    ivl_net_const_t net = ivl_design_const(des, idx);
	    const char*val = ivl_const_bits(net);

	    for (pin = 0 ;  pin < ivl_const_pins(net) ;  pin += 1) {
		  ivl_nexus_t nex = ivl_const_pin(net, pin);
		  const char*name;
		  const char*port;

		  edif_uref += 1;

		  switch (val[pin]) {
		      case '0':
			name = "GND";
			port = "GROUND";
			break;
		      case '1':
			name = "VCC";
			port = "VCC";
			break;
		      default:
			name = "???";
			port = "?";
			break;
		  }

		  fprintf(xnf, "(instance U%u "
			  "(viewRef net"
			  " (cellRef %s (libraryRef VIRTEX))))\n",
			  edif_uref, name);

		  sprintf(jbuf, "(portRef %s (instanceRef U%u))",
			  port, edif_uref);
		  edif_set_nexus_joint(nex, jbuf);
	    }
      }

}

static void edif_show_footer(ivl_design_t des)
{
      unsigned nref = 0;
      struct nexus_recall*cur;
      ivl_scope_t root = ivl_design_root(des);

      edif_show_consts(des);

      for (cur = net_list ;  cur ;  cur = cur->next) {
	    fprintf(xnf, "(net (rename N%u \"%s\") (joined %s))\n",
		    nref, ivl_nexus_name(cur->nex), cur->joined);
	    nref += 1;
      }

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
      char jbuf[1024];
      unsigned idx;

      edif_uref += 1;

      switch (ivl_logic_type(net)) {

	  case IVL_LO_AND:
	    assert(ivl_logic_pins(net) <= 10);
	    assert(ivl_logic_pins(net) >= 3);

	    fprintf(xnf, "(instance (rename U%u \"%s\")",
		    edif_uref, ivl_logic_name(net));
	    fprintf(xnf, " (viewRef net"
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
	    fprintf(xnf, " (viewRef net"
		    " (cellRef BUF (libraryRef VIRTEX))))\n");

	    sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 0), jbuf);

	    sprintf(jbuf, "(portRef I (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 1), jbuf);
	    break;

        case IVL_LO_BUFZ:
          {
            static int bufz_warned_once=0;
            if (!bufz_warned_once) {
              fprintf (stderr,
                       "0:0: internal warning: BUFZ objects found "
                       "in EDIF netlist.\n");
              fprintf (stderr,
                       "0:0:                 : I'll make BUFs for them.\n");
              bufz_warned_once=1;
            }
            assert(ivl_logic_pins(net) == 2);
            fprintf(xnf, "(instance (rename U%u \"%s\")",
                    edif_uref, ivl_logic_name(net));
            fprintf(xnf, " (viewRef net"
                    " (cellRef BUF (libraryRef VIRTEX))))\n");

            sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
            edif_set_nexus_joint(ivl_logic_pin(net, 0), jbuf);

            sprintf(jbuf, "(portRef I (instanceRef U%u))", edif_uref);
            edif_set_nexus_joint(ivl_logic_pin(net, 1), jbuf);
          }
          break;

	  case IVL_LO_NOR:
	    assert(ivl_logic_pins(net) <= 10);
	    assert(ivl_logic_pins(net) >= 3);

	    fprintf(xnf, "(instance (rename U%u \"%s\")",
		    edif_uref, ivl_logic_name(net));
	    fprintf(xnf, " (viewRef net"
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
	    fprintf(stderr, "UNSUPPORT LOGIC TYPE: %d\n", ivl_logic_type(net));
      }
}

static void edif_show_generic_dff(ivl_lpm_t net)
{
      char jbuf[1024];
      unsigned idx;
      ivl_nexus_t aclr = ivl_lpm_async_clr(net);
      ivl_nexus_t aset = ivl_lpm_async_set(net);
      const char*abits = 0;
      const char*fdcell = "FDCE";

      if (aset != 0) {
	    ivl_expr_t avalue = ivl_lpm_aset_value(net);
	    fdcell = "FDCPE";
	    assert(avalue);
	    abits = ivl_expr_bits(avalue);
	    assert(abits);
      }

      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    ivl_nexus_t nex;

	    edif_uref += 1;

	    fprintf(xnf, "(instance (rename U%u \"%s.%s[%u]\")",
		    edif_uref, ivl_scope_name(ivl_lpm_scope(net)),
		    ivl_lpm_basename(net), idx);
	    fprintf(xnf, " (viewRef net"
		    " (cellRef %s (libraryRef VIRTEX))))\n",
		    fdcell);

	    nex = ivl_lpm_q(net, idx);
	    sprintf(jbuf, "(portRef Q (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(nex, jbuf);

	    nex = ivl_lpm_data(net, idx);
	    sprintf(jbuf, "(portRef D (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(nex, jbuf);

	    nex = ivl_lpm_clk(net);
	    sprintf(jbuf, "(portRef C (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(nex, jbuf);

	    if ((nex = ivl_lpm_enable(net))) {
		  sprintf(jbuf, "(portRef CE (instanceRef U%u))", edif_uref);
		  edif_set_nexus_joint(nex, jbuf);
	    }

	    if (aclr) {
		  sprintf(jbuf, "(portRef CLR (instanceRef U%u))", edif_uref);
		  edif_set_nexus_joint(aclr, jbuf);
	    }


	    if (aset) {
	       if (abits[idx] == '1') {
		     sprintf(jbuf, "(portRef PRE (instanceRef U%u))",
			     edif_uref);
		     edif_set_nexus_joint(aset, jbuf);
	       } else {
		     assert(aclr == 0);
		     sprintf(jbuf, "(portRef CLR (instanceRef U%u))",
			     edif_uref);
		     edif_set_nexus_joint(aset, jbuf);
	       }
	    }
      }
}


const struct device_s d_generic_edif = {
      edif_show_header,
      edif_show_footer,
      0, /* show_cell_scope not implemented. */
      0, /* draw_pad not implemented */
      edif_show_logic,
      edif_show_generic_dff,
      0, /* show_cmp_eq */
      0, /* show_cmp_ne */
      0, /* show_cmp_ge */
      0, /* show_cmp_gt */
      0,
      0, /* show_add */
      0, /* show_sub */
      0, /* show_shiftl */
      0  /* show_shiftr */
};
