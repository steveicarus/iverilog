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
#ident "$Id: d-virtex.c,v 1.4 2001/09/11 05:52:31 steve Exp $"

# include  "device.h"
# include  "fpga_priv.h"
# include  <stdlib.h>
# include  <string.h>
# include  <malloc.h>
# include  <assert.h>

/*
 * This is the EDIF code generator for VIRTEX style parts. It uses the
 * following VIRTEX primitives from the unified library.
 *
 *   BUF        O, I
 *     non-inverting buffer. This device is typically removed by the
 *     place-and-route step, as it is not normally needed within an
 *     FPGA net.
 *
 *   INV        O, I
 *     Inverting buffer.
 *
 *   LUT2       O, I0, I1
 *   LUT3       O, I0, I1, I2
 *   LUT4       O, I0, I1, I2, I3
 *     These are look-up tables. They represent the LUT sub-devices
 *     that live in the CLBs, 2 per slice. The logic value of the
 *     device itself is given by an INIT property.
 *
 *     The INIT property is a string of hex digits. The binary value
 *     that the digits represents is the outputs addressed by the
 *     inputs. For example, to get an AND2 from LUT2, INIT=8.
 *
 *   MUXCY_L    LO, S, DI, CI
 *
 *   XORCY      O, LI, CI
 */

static const char*virtex_library_text =
"    (external VIRTEX (edifLevel 0) (technology (numberDefinition))\n"
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
"      (cell INV (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I (direction INPUT)))))\n"
"      (cell LUT2 (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT)))))\n"
"      (cell LUT3 (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT))\n"
"                 (port I2 (direction INPUT)))))\n"
"      (cell LUT4 (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port I0 (direction INPUT))\n"
"                 (port I1 (direction INPUT))\n"
"                 (port I2 (direction INPUT))\n"
"                 (port I3 (direction INPUT)))))\n"
"      (cell MUXCY (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port S (direction INPUT))\n"
"                 (port DI (direction INPUT))\n"
"                 (port CI (direction INPUT)))))\n"
"      (cell MUXCY_L (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port LO (direction OUTPUT))\n"
"                 (port S (direction INPUT))\n"
"                 (port DI (direction INPUT))\n"
"                 (port CI (direction INPUT)))))\n"
"      (cell VCC (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface (port P (direction OUTPUT)))))\n"
"      (cell XORCY (cellType GENERIC)\n"
"            (view Netlist_representation\n"
"              (viewType NETLIST)\n"
"              (interface\n"
"                 (port O (direction OUTPUT))\n"
"                 (port LI (direction INPUT))\n"
"                 (port CI (direction INPUT)))))\n"
"    )\n"
;


static void edif_show_header(ivl_design_t des)
{
      edif_show_header_generic(des, virtex_library_text);
}

static void edif_show_lut2(const char*name, unsigned uref,
			   ivl_nexus_t O, ivl_nexus_t I0, ivl_nexus_t I1,
			   const char*truth_table)
{
      char jbuf[1024];

      fprintf(xnf, "(instance (rename U%u \"%s\")"
	      " (viewRef Netlist_representation"
	      " (cellRef LUT2 (libraryRef VIRTEX)))"
	      " (property INIT (string \"%s\")))\n",
	      uref, name, truth_table);

      sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(O, jbuf);

      sprintf(jbuf, "(portRef I0 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I0, jbuf);

      sprintf(jbuf, "(portRef I1 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I1, jbuf);
}

static void edif_show_lut3(const char*name, unsigned uref,
			   ivl_nexus_t O,
			   ivl_nexus_t I0,
			   ivl_nexus_t I1,
			   ivl_nexus_t I2,
			   const char*truth_table)
{
      char jbuf[1024];

      fprintf(xnf, "(instance (rename U%u \"%s\")"
	      " (viewRef Netlist_representation"
	      " (cellRef LUT3 (libraryRef VIRTEX)))"
	      " (property INIT (string \"%s\")))\n",
	      uref, name, truth_table);

      sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(O, jbuf);

      sprintf(jbuf, "(portRef I0 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I0, jbuf);

      sprintf(jbuf, "(portRef I1 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I1, jbuf);

      sprintf(jbuf, "(portRef I2 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I2, jbuf);
}

static void edif_show_lut4(const char*name, unsigned uref,
			   ivl_nexus_t O,
			   ivl_nexus_t I0, ivl_nexus_t I1,
			   ivl_nexus_t I2, ivl_nexus_t I3,
			   const char*truth_table)
{
      char jbuf[1024];

      fprintf(xnf, "(instance (rename U%u \"%s\")"
	      " (viewRef Netlist_representation"
	      " (cellRef LUT4 (libraryRef VIRTEX)))"
	      " (property INIT (string \"%s\")))\n",
	      uref, name, truth_table);

      sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(O, jbuf);

      sprintf(jbuf, "(portRef I0 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I0, jbuf);

      sprintf(jbuf, "(portRef I1 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I1, jbuf);

      sprintf(jbuf, "(portRef I2 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I2, jbuf);

      sprintf(jbuf, "(portRef I3 (instanceRef U%u))", edif_uref);
      edif_set_nexus_joint(I3, jbuf);
}

static void edif_show_virtex_logic(ivl_net_logic_t net)
{
      char jbuf[1024];

      edif_uref += 1;

      switch (ivl_logic_type(net)) {

	  case IVL_LO_AND:
	    assert(ivl_logic_pins(net) <= 5);
	    assert(ivl_logic_pins(net) >= 3);

	    switch (ivl_logic_pins(net)) {
		case 3:
		  edif_show_lut2(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2), "8");
		  break;
		case 4:
		  edif_show_lut3(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2),
				 ivl_logic_pin(net, 3), "80");
		  break;
		case 5:
		  edif_show_lut4(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2),
				 ivl_logic_pin(net, 3),
				 ivl_logic_pin(net, 4), "8000");
		  break;
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
	    assert(ivl_logic_pins(net) <= 5);
	    assert(ivl_logic_pins(net) >= 3);

	    switch (ivl_logic_pins(net)) {
		case 3:
		  edif_show_lut2(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2), "1");
		  break;
		case 4:
		  edif_show_lut3(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2),
				 ivl_logic_pin(net, 3), "01");
		  break;
		case 5:
		  edif_show_lut4(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2),
				 ivl_logic_pin(net, 3),
				 ivl_logic_pin(net, 4), "0001");
		  break;
	    }
	    break;

	  case IVL_LO_NOT:
	    assert(ivl_logic_pins(net) == 2);
	    fprintf(xnf, "(instance (rename U%u \"%s\")",
		    edif_uref, ivl_logic_name(net));
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef INV (libraryRef VIRTEX))))\n");

	    sprintf(jbuf, "(portRef O (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 0), jbuf);

	    sprintf(jbuf, "(portRef I (instanceRef U%u))", edif_uref);
	    edif_set_nexus_joint(ivl_logic_pin(net, 1), jbuf);
	    break;

	  case IVL_LO_OR:
	    assert(ivl_logic_pins(net) <= 5);
	    assert(ivl_logic_pins(net) >= 3);

	    switch (ivl_logic_pins(net)) {
		case 3:
		  edif_show_lut2(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2), "E");
		  break;
		case 4:
		  edif_show_lut3(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2),
				 ivl_logic_pin(net, 3), "FE");
		  break;
		case 5:
		  edif_show_lut4(ivl_logic_name(net), edif_uref,
				 ivl_logic_pin(net, 0),
				 ivl_logic_pin(net, 1),
				 ivl_logic_pin(net, 2),
				 ivl_logic_pin(net, 3),
				 ivl_logic_pin(net, 4), "FFFE");
		  break;
	    }
	    break;

	  default:
	    fprintf(stderr, "UNSUPPORTED LOGIC TYPE: %u\n",
		    ivl_logic_type(net));
      }
}

static void edif_show_virtex_eq(ivl_lpm_t net)
{
      assert(ivl_lpm_width(net) >= 1);

      edif_uref += 1;

      switch (ivl_lpm_width(net)) {
	  case 1:
	    edif_show_lut2(ivl_lpm_name(net), edif_uref,
			   ivl_lpm_q(net, 0),
			   ivl_lpm_data(net, 0),
			   ivl_lpm_datab(net, 0), "9");
	    break;

	  case 2:
	    edif_show_lut4(ivl_lpm_name(net), edif_uref,
			   ivl_lpm_q(net, 0),
			   ivl_lpm_data(net, 0), ivl_lpm_datab(net, 0),
			   ivl_lpm_data(net, 1), ivl_lpm_datab(net, 1),
			   "9009");
	    break;

	  default: {
		char jbuf[1024];
		unsigned idx;
		unsigned pairs = ivl_lpm_width(net) / 2;
		unsigned tail  = ivl_lpm_width(net) % 2;

		if (tail == 0) {
		      tail = 2;
		      pairs -= 1;
		}

		fprintf(xnf, "(instance U%uL0"
			" (viewRef Netlist_representation"
			" (cellRef LUT4 (libraryRef VIRTEX)))"
			" (property INIT (string \"9009\")))\n",
			edif_uref);
		fprintf(xnf, "(instance U%uM0"
			" (viewRef Netlist_representation"
			" (cellRef MUXCY_L (libraryRef VIRTEX))))\n",
			edif_uref);
		fprintf(xnf, "(instance U%uG0"
			" (viewRef Netlist_representation"
			" (cellRef GND (libraryRef VIRTEX))))\n",
			edif_uref);
		fprintf(xnf, "(instance U%uV0"
			" (viewRef Netlist_representation"
			" (cellRef VCC (libraryRef VIRTEX))))\n",
			edif_uref);

		fprintf(xnf, "(net U%uVM0 (joined"
			" (portRef P (instanceRef U%uV0))"
			" (portRef CI (instanceRef U%uM0))))\n",
			edif_uref, edif_uref, edif_uref);
		fprintf(xnf, "(net U%uGM0 (joined"
			" (portRef G (instanceRef U%uG0))"
			" (portRef DI (instanceRef U%uM0))))\n",
			edif_uref, edif_uref, edif_uref);
		fprintf(xnf, "(net U%uLM0 (joined"
			" (portRef O (instanceRef U%uL0))"
			" (portRef S (instanceRef U%uM0))))\n",
			edif_uref, edif_uref, edif_uref);

		sprintf(jbuf, "(portRef I0 (instanceRef U%uL0))", edif_uref);
		edif_set_nexus_joint(ivl_lpm_data(net, 0), jbuf);

		sprintf(jbuf, "(portRef I1 (instanceRef U%uL0))", edif_uref);
		edif_set_nexus_joint(ivl_lpm_datab(net, 0), jbuf);

		sprintf(jbuf, "(portRef I2 (instanceRef U%uL0))", edif_uref);
		edif_set_nexus_joint(ivl_lpm_data(net, 1), jbuf);

		sprintf(jbuf, "(portRef I3 (instanceRef U%uL0))", edif_uref);
		edif_set_nexus_joint(ivl_lpm_datab(net, 1), jbuf);


		for (idx = 1 ;  idx < pairs ;  idx += 1) {
		      fprintf(xnf, "(instance U%uL%u"
			      " (viewRef Netlist_representation"
			      " (cellRef LUT4 (libraryRef VIRTEX)))"
			      " (property INIT (string \"9009\")))\n",
			      edif_uref, idx);
		      fprintf(xnf, "(instance U%uM%u"
			      " (viewRef Netlist_representation"
			      " (cellRef MUXCY_L (libraryRef VIRTEX))))\n",
			      edif_uref, idx);
		      fprintf(xnf, "(instance U%uG%u"
			      " (viewRef Netlist_representation"
			      " (cellRef GND (libraryRef VIRTEX))))\n",
			      edif_uref, idx);

		      fprintf(xnf, "(net U%uVM%u (joined"
			      " (portRef LO (instanceRef U%uM%u))"
			      " (portRef CI (instanceRef U%uM%u))))\n",
			      edif_uref, idx, edif_uref, idx-1,
			      edif_uref, idx);
		      fprintf(xnf, "(net U%uGM%u (joined"
			      " (portRef G (instanceRef U%uG%u))"
			      " (portRef DI (instanceRef U%uM%u))))\n",
			      edif_uref, idx, edif_uref, idx,
			      edif_uref, idx);
		      fprintf(xnf, "(net U%uLM%u (joined"
			      " (portRef O (instanceRef U%uL%u))"
			      " (portRef S (instanceRef U%uM%u))))\n",
			      edif_uref, idx, edif_uref, idx,
			      edif_uref, idx);

		      sprintf(jbuf, "(portRef I0 (instanceRef U%uL%u))",
			      edif_uref, idx);
		      edif_set_nexus_joint(ivl_lpm_data(net, idx*2), jbuf);

		      sprintf(jbuf, "(portRef I1 (instanceRef U%uL%u))",
			      edif_uref, idx);
		      edif_set_nexus_joint(ivl_lpm_datab(net, idx*2), jbuf);

		      sprintf(jbuf, "(portRef I2 (instanceRef U%uL%u))",
			      edif_uref, idx);
		      edif_set_nexus_joint(ivl_lpm_data(net, idx*2+1), jbuf);

		      sprintf(jbuf, "(portRef I3 (instanceRef U%uL%u))",
			      edif_uref, idx);
		      edif_set_nexus_joint(ivl_lpm_datab(net, idx*2+1), jbuf);

		}

		if (tail == 2) {
		      fprintf(xnf, "(instance U%uL%u"
			      " (viewRef Netlist_representation"
			      " (cellRef LUT4 (libraryRef VIRTEX)))"
			      " (property INIT (string \"9009\")))\n",
			      edif_uref, pairs);
		      fprintf(xnf, "(instance (rename U%uM%u \"%s\")"
			      " (viewRef Netlist_representation"
			      " (cellRef MUXCY (libraryRef VIRTEX))))\n",
			      edif_uref, pairs, ivl_lpm_name(net));
		      fprintf(xnf, "(instance U%uG%u"
			      " (viewRef Netlist_representation"
			      " (cellRef GND (libraryRef VIRTEX))))\n",
			      edif_uref, pairs);

		      fprintf(xnf, "(net U%uVM%u (joined"
			      " (portRef LO (instanceRef U%uM%u))"
			      " (portRef CI (instanceRef U%uM%u))))\n",
			      edif_uref, pairs, edif_uref, pairs-1,
			      edif_uref, pairs);
		      fprintf(xnf, "(net U%uGM%u (joined"
			      " (portRef G (instanceRef U%uG%u))"
			      " (portRef DI (instanceRef U%uM%u))))\n",
			      edif_uref, pairs, edif_uref, pairs,
			      edif_uref, pairs);
		      fprintf(xnf, "(net U%uLM%u (joined"
			      " (portRef O (instanceRef U%uL%u))"
			      " (portRef S (instanceRef U%uM%u))))\n",
			      edif_uref, pairs, edif_uref, pairs,
			      edif_uref, pairs);

		      sprintf(jbuf, "(portRef I0 (instanceRef U%uL%u))",
			      edif_uref, pairs);
		      edif_set_nexus_joint(ivl_lpm_data(net, pairs*2), jbuf);

		      sprintf(jbuf, "(portRef I1 (instanceRef U%uL%u))",
			      edif_uref, pairs);
		      edif_set_nexus_joint(ivl_lpm_datab(net, pairs*2), jbuf);

		      sprintf(jbuf, "(portRef I2 (instanceRef U%uL%u))",
			      edif_uref, pairs);
		      edif_set_nexus_joint(ivl_lpm_data(net, pairs*2+1), jbuf);

		      sprintf(jbuf, "(portRef I3 (instanceRef U%uL%u))",
			      edif_uref, pairs);
		      edif_set_nexus_joint(ivl_lpm_datab(net, pairs*2+1), jbuf);

		} else {
		      assert(tail == 1);

		      fprintf(xnf, "(instance (rename U%uL%u \"%s\")"
			      " (viewRef Netlist_representation"
			      " (cellRef LUT2 (libraryRef VIRTEX)))"
			      " (property INIT (string \"9\")))\n",
			      edif_uref, pairs, ivl_lpm_name(net));
		      fprintf(xnf, "(instance U%uM%u"
			      " (viewRef Netlist_representation"
			      " (cellRef MUXCY (libraryRef VIRTEX))))\n",
			      edif_uref, pairs);
		      fprintf(xnf, "(instance U%uG%u"
			      " (viewRef Netlist_representation"
			      " (cellRef GND (libraryRef VIRTEX))))\n",
			      edif_uref, pairs);

		      fprintf(xnf, "(net U%uVM%u (joined"
			      " (portRef LO (instanceRef U%uM%u))"
			      " (portRef CI (instanceRef U%uM%u))))\n",
			      edif_uref, pairs, edif_uref, pairs-1,
			      edif_uref, pairs);
		      fprintf(xnf, "(net U%uGM%u (joined"
			      " (portRef G (instanceRef U%uG%u))"
			      " (portRef DI (instanceRef U%uM%u))))\n",
			      edif_uref, pairs, edif_uref, pairs,
			      edif_uref, pairs);
		      fprintf(xnf, "(net U%uLM%u (joined"
			      " (portRef O (instanceRef U%uL%u))"
			      " (portRef S (instanceRef U%uM%u))))\n",
			      edif_uref, pairs, edif_uref, pairs,
			      edif_uref, pairs);

		      sprintf(jbuf, "(portRef I0 (instanceRef U%uL%u))",
			      edif_uref, pairs);
		      edif_set_nexus_joint(ivl_lpm_data(net, pairs*2), jbuf);

		      sprintf(jbuf, "(portRef I1 (instanceRef U%uL%u))",
			      edif_uref, pairs);
		      edif_set_nexus_joint(ivl_lpm_datab(net, pairs*2), jbuf);

		}

		sprintf(jbuf, "(portRef O (instanceRef U%uL%u))",
			edif_uref, pairs);
		edif_set_nexus_joint(ivl_lpm_q(net, 0), jbuf);

		break;
	  }
      }
}

/*
 * This supports the general MUX with a single select input. The
 * output is selected from one of two inputs.
 *
 * This implements the mux a bit slice at a time. Each slice is a
 * 1-bit mux implemented with a three input LUT: I0 and I1 are the
 * alternative inputs, and I2 is the select.
 *
 * FIXME: In the long run, it would be cool to detect that the inputs
 * of the mux are themselves LUT devices and generate MUXF5 devices in
 * those cases. This currently does *not* do that.
 */
static void edif_show_virtex_mux(ivl_lpm_t net)
{
      unsigned idx;
      assert(ivl_lpm_width(net) >= 1);
      assert(ivl_lpm_selects(net) == 1);

      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    char tmp_name[1024];

	    edif_uref += 1;
	    sprintf(tmp_name, "%s<%u>", ivl_lpm_name(net), idx);

	    edif_show_lut3(tmp_name, edif_uref,
			   ivl_lpm_q(net, idx),
			   ivl_lpm_data2(net, 0, idx),
			   ivl_lpm_data2(net, 1, idx),
			   ivl_lpm_select(net, 0),
			   "CA");
      }
}

static void edif_show_virtex_add(ivl_lpm_t net)
{
      char jbuf [1024];
      unsigned idx;
      unsigned nref = 0;

	/* Handle the special case that the adder is only one bit
	   wide. Generate an XOR gate to perform the half-add. */
      if (ivl_lpm_width(net) == 1) {
	    edif_uref += 1;

	    edif_show_lut2(ivl_lpm_name(net), edif_uref,
			   ivl_lpm_q(net, 0),
			   ivl_lpm_data(net, 0),
			   ivl_lpm_datab(net, 0),
			   "6");
	    return;
      }

      assert(ivl_lpm_width(net) > 1);
      edif_uref += 1;

	/* First, draw the bottom bit slice of the adder. This
	   includes the LUT2 device to perform the addition, and a
	   MUXCY_L device to send the carry up to the next bit. */
      fprintf(xnf, "(instance (rename U%u_L0 \"%s\"[0])"
	      " (property INIT (string \"6\"))", edif_uref,
	      ivl_lpm_name(net));
      fprintf(xnf, " (viewRef Netlist_representation"
	      " (cellRef LUT2 (libraryRef VIRTEX))))\n");

      fprintf(xnf, "(instance U%u_M0", edif_uref);
      fprintf(xnf, " (viewRef Netlist_representation"
	      " (cellRef MUXCY_L (libraryRef VIRTEX))))\n");

      sprintf(jbuf, "(portRef I0 (instanceRef U%u_L0))", edif_uref);
      edif_set_nexus_joint(ivl_lpm_data(net, 0), jbuf);

      sprintf(jbuf, "(portRef I1 (instanceRef U%u_L0))", edif_uref);
      edif_set_nexus_joint(ivl_lpm_datab(net, 0), jbuf);

      sprintf(jbuf, "(portRef O (instanceRef U%u_L0))"
	      " (portRef S (instanceRef U%u_M0))",
	      edif_uref, edif_uref);
      edif_set_nexus_joint(ivl_lpm_q(net, 0), jbuf);

	/* Now draw all the inside bit slices. These include the LUT2
	   device for the basic add, the MUXCY_L device to propagate
	   the carry, and an XORCY device to generate the real
	   output. The XORCY device carries the name of the LPM
	   device, the other devices have local names. */
      for (idx = 1 ;  idx < (ivl_lpm_width(net)-1) ; idx += 1) {

	    fprintf(xnf, "(instance U%u_L%u) (property INIT (string \"6\"))",
		    edif_uref, idx);
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef LUT2 (libraryRef VIRTEX))))\n");

	    fprintf(xnf, "(instance U%u_M%u", edif_uref, idx);
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef MUXCY_L (libraryRef VIRTEX))))\n");

	    fprintf(xnf, "(instance (rename U%u_X%u \"%s[%u]\")",
		    edif_uref, idx, ivl_lpm_name(net), idx);
	    fprintf(xnf, " (viewRef Netlist_representation"
		    " (cellRef XORCY (libraryRef VIRTEX))))\n");

	    fprintf(xnf, "(net U%uN%u (joined"
		    " (portRef O (instanceRef U%u_L%u))"
		    " (portRef S (instanceRef U%u_M%u))"
		    " (portRef LI (instanceRef U%u_X%u))))\n",
		    edif_uref, nref++, edif_uref, idx, edif_uref, idx,
		    edif_uref, idx);

	    fprintf(xnf, "(net U%uN%u (joined"
		    " (portRef CI (instanceRef U%u_M%u))"
		    " (portRef CI (instanceRef U%u_X%u))"
		    " (portRef LO (instanceRef U%u_M%u))))\n",
		    edif_uref, nref++, edif_uref, idx, edif_uref, idx,
		    edif_uref, idx-1);

	    sprintf(jbuf, "(portRef I0 (instanceRef U%u_L%u))",
		    edif_uref, idx);
	    edif_set_nexus_joint(ivl_lpm_data(net, idx), jbuf);

	    sprintf(jbuf, "(portRef I1 (instanceRef U%u_L%u))",
		    edif_uref, idx);
	    edif_set_nexus_joint(ivl_lpm_datab(net, idx), jbuf);

	    sprintf(jbuf, "(portRef O (instanceRef U%u_X%u))",
		    edif_uref, idx);
	    edif_set_nexus_joint(ivl_lpm_q(net, idx), jbuf);
      }


      fprintf(xnf, "(instance U%u_L%u) (property INIT (string \"6\"))",
	      edif_uref, idx);
      fprintf(xnf, " (viewRef Netlist_representation"
	      " (cellRef LUT2 (libraryRef VIRTEX))))\n");

      fprintf(xnf, "(instance (rename U%u_X%u \"%s[%u]\")",
	      edif_uref, idx, ivl_lpm_name(net), idx);
      fprintf(xnf, " (viewRef Netlist_representation"
	      " (cellRef XORCY (libraryRef VIRTEX))))\n");

      fprintf(xnf, "(net U%uN%u (joined"
	      " (portRef O (instanceRef U%u_L%u))"
	      " (portRef LI (instanceRef U%u_X%u))))\n",
	      edif_uref, nref++, edif_uref, idx, edif_uref, idx);

      fprintf(xnf, "(net U%uN%u (joined"
	      " (portRef CI (instanceRef U%u_X%u))"
	      " (portRef LO (instanceRef U%u_M%u))))\n",
	      edif_uref, nref++, edif_uref, idx, edif_uref, idx-1);

      sprintf(jbuf, "(portRef I0 (instanceRef U%u_L%u))",
	      edif_uref, idx);
      edif_set_nexus_joint(ivl_lpm_data(net, idx), jbuf);

      sprintf(jbuf, "(portRef I1 (instanceRef U%u_L%u))",
	      edif_uref, idx);
      edif_set_nexus_joint(ivl_lpm_datab(net, idx), jbuf);

      sprintf(jbuf, "(portRef O (instanceRef U%u_X%u))",
	      edif_uref, idx);
      edif_set_nexus_joint(ivl_lpm_q(net, idx), jbuf);
}

const struct device_s d_virtex_edif = {
      edif_show_header,
      edif_show_footer,
      edif_show_virtex_logic,
      edif_show_generic_dff,
      edif_show_virtex_eq,
      0,
      edif_show_virtex_mux,
      edif_show_virtex_add
};


/*
 * $Log: d-virtex.c,v $
 * Revision 1.4  2001/09/11 05:52:31  steve
 *  Use carry mux to implement wide identity compare,
 *  Place property item in correct place in LUT cell list.
 *
 * Revision 1.3  2001/09/10 03:48:34  steve
 *  Add 4 wide identity compare.
 *
 * Revision 1.2  2001/09/09 22:23:28  steve
 *  Virtex support for mux devices and adders
 *  with carry chains. Also, make Virtex specific
 *  implementations of primitive logic.
 *
 * Revision 1.1  2001/09/06 04:28:40  steve
 *  Separate the virtex and generic-edif code generators.
 *
 */

