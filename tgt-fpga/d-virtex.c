/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: d-virtex.c,v 1.25 2003/06/25 02:55:57 steve Exp $"
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
const static struct edif_xlib_celltable virtex_celltable[] = {
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
static void virtex_show_header(ivl_design_t des)
{
      unsigned idx;
      ivl_scope_t root = ivl_design_root(des);
      unsigned sig_cnt = ivl_scope_sigs(root);
      unsigned nports = 0, pidx;
      const char*part_str = 0;

	/* Count the ports I'm going to use. */
      for (idx = 0 ;  idx < sig_cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);

	    if (ivl_signal_port(sig) == IVL_SIP_NONE)
		  continue;

	    if (ivl_signal_attr(sig, "PAD") != 0)
		  continue;

	    nports += ivl_signal_pins(sig);
      }

      edf = edif_create(ivl_scope_basename(root), nports);

      pidx = 0;
      for (idx = 0 ;  idx < sig_cnt ;  idx += 1) {
	    edif_joint_t jnt;
	    ivl_signal_t sig = ivl_scope_sig(root, idx);

	    if (ivl_signal_port(sig) == IVL_SIP_NONE)
		  continue;

	    if (ivl_signal_attr(sig, "PAD") != 0)
		  continue;

	    if (ivl_signal_pins(sig) == 1) {
		  edif_portconfig(edf, pidx, ivl_signal_basename(sig),
				  ivl_signal_port(sig));

		  assert(ivl_signal_pins(sig) == 1);
		  jnt = edif_joint_of_nexus(edf, ivl_signal_pin(sig, 0));
		  edif_port_to_joint(jnt, edf, pidx);

	    } else {
		  const char*name = ivl_signal_basename(sig);
		  ivl_signal_port_t dir = ivl_signal_port(sig);
		  char buf[128];
		  unsigned bit;
		  for (bit = 0 ;  bit < ivl_signal_pins(sig) ; bit += 1) {
			const char*tmp;
			sprintf(buf, "%s[%u]", name, bit);
			tmp = strdup(buf);
			edif_portconfig(edf, pidx+bit, tmp, dir);

			jnt = edif_joint_of_nexus(edf,ivl_signal_pin(sig,bit));
			edif_port_to_joint(jnt, edf, pidx+bit);
		  }
	    }

	    pidx += ivl_signal_pins(sig);
      }

      assert(pidx == nports);

      xlib = edif_xlibrary_create(edf, "VIRTEX");
      edif_xlibrary_set_celltable(xlib, virtex_celltable);


      if ( (part_str = ivl_design_flag(des, "part")) && (part_str[0] != 0) ) {
	    edif_pstring(edf, "PART", part_str);
      }

      cell_0 = edif_xcell_create(xlib, "GND", 1);
      edif_cell_portconfig(cell_0, 0, "GROUND", IVL_SIP_OUTPUT);

      cell_1 = edif_xcell_create(xlib, "VCC", 1);
      edif_cell_portconfig(cell_1, 0, "VCC", IVL_SIP_OUTPUT);

}

void virtex_show_footer(ivl_design_t des)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_design_consts(des) ;  idx += 1) {
	    unsigned pin;
	    ivl_net_const_t net = ivl_design_const(des, idx);
	    const char*val = ivl_const_bits(net);

	    for (pin = 0 ;  pin < ivl_const_pins(net) ;  pin += 1) {
		  edif_joint_t jnt;
		  edif_cellref_t pad;

		  jnt = edif_joint_of_nexus(edf, ivl_const_pin(net, pin));
		  switch (val[pin]) {
		      case '0':
			pad = edif_cellref_create(edf, cell_0);
			break;
		      case '1':
			pad = edif_cellref_create(edf, cell_1);
			break;
		      default:
			assert(0);
			break;
		  }

		  edif_add_to_joint(jnt, pad, 0);
	    }
      }

      edif_print(xnf, edf);
}

void virtex_generic_dff(ivl_lpm_t net)
{
      unsigned idx;

      ivl_nexus_t aclr = ivl_lpm_async_clr(net);
      ivl_nexus_t aset = ivl_lpm_async_set(net);
      const char*abits = 0;

      if (aset) {
	    ivl_expr_t avalue = ivl_lpm_aset_value(net);
	    assert(avalue);
	    abits = ivl_expr_bits(avalue);
	    assert(abits);
      }


      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    edif_cellref_t obj;
	    ivl_nexus_t nex;
	    edif_joint_t jnt;

	      /* If there is a preset, then select an FDCPE instead of
		 an FDCE device. */
	    if (aset && (abits[idx] == '1')) {
		  obj = edif_cellref_create(edf, xilinx_cell_fdcpe(xlib));
	    } else {
		  obj = edif_cellref_create(edf, xilinx_cell_fdce(xlib));
	    }

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, idx));
	    edif_add_to_joint(jnt, obj, FDCE_Q);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
	    edif_add_to_joint(jnt, obj, FDCE_D);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_clk(net));
	    edif_add_to_joint(jnt, obj, FDCE_C);

	    if ( (nex = ivl_lpm_enable(net)) ) {
		  jnt = edif_joint_of_nexus(edf, nex);
		  edif_add_to_joint(jnt, obj, FDCE_CE);
	    }

	    if (aclr) {
		  jnt = edif_joint_of_nexus(edf, aclr);
		  edif_add_to_joint(jnt, obj, FDCE_CLR);
	    }

	    if (aset) {
		  if (abits[idx] == '1') {
			jnt = edif_joint_of_nexus(edf, aset);
			edif_add_to_joint(jnt, obj, FDCE_PRE);
		  } else {
			assert(aclr == 0);
			jnt = edif_joint_of_nexus(edf, aset);
			edif_add_to_joint(jnt, obj, FDCE_CLR);
		  }
	    }
      }
}

/*
 * This method handles both == and != operators, the identity
 * comparison operators.
 *
 * If the identity compare is applied to small enough input vectors,
 * it is shoved into a single LUT. Otherwise, it is strung out into a
 * row of LUT devices chained together by carry muxes. The output of
 * the comparison is the output of the last mux.
 *
 * When the compare is small, a LUT is generated with the appropriate
 * truth table to cause an == or != result.
 *
 * When the compare is too wide for a single LUT, then it is made into
 * a chain connected by a string of carry mux devices. Each LUT
 * implements == for up to two pairs of bits, even if the final output
 * is supposed to be !=. The LUT output is connected to an associated
 * MUX select input. The CO output of each muxcy is passed up to the
 * next higher order bits of the compare.
 *
 * For identity == compare, a != output from the LUT selects the DI
 * input of the muxcy, generating a 0 output that is passed up. Since
 * the next higher muxcy now gets a 0 input to both DI and CI, the
 * output of the next higher muxcy is guaranteed to be 0, and so on to
 * the final output of the carry chain. If the output from a LUT is ==,
 * then the CI input of the muxcy is selected and the truth of this
 * level depends on lower order bits. The least significant muxcy is
 * connected to GND and VCC so that its CO follows the least
 * significant LUT.
 *
 * Identity != is the same as == except that the output is
 * inverted. To get that effect without putting an inverter on the
 * output of the top muxcy pin CO (which would cost a LUT) the DI
 * inputs are all connected to VCC instead of GND, and the CI of the
 * least significant muxcy is connected to GND instead of VCC. The LUT
 * expressions for the chained compare are configured for ==, with the
 * changed CI/DI inputs performing the inversion.
 */
void virtex_eq(ivl_lpm_t net)
{
      edif_cellref_t lut, mux, mux_prev;
      edif_joint_t jnt, jnt_di;
      unsigned idx;

	/* True if I'm implementing CMP_EQ instead of CMP_NE */
      int eq = 1;

      assert(ivl_lpm_width(net) >= 1);

      if (ivl_lpm_type(net) == IVL_LPM_CMP_NE)
	    eq = 0;

      switch (ivl_lpm_width(net)) {

	  case 1:
	    lut = edif_cellref_create(edf, xilinx_cell_lut2(xlib));
	    edif_cellref_pstring(lut, "INIT", eq? "9" : "6");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_O);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_I0);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_I1);
	    return;

	  case 2:
	    lut = edif_cellref_create(edf, xilinx_cell_lut4(xlib));
	    edif_cellref_pstring(lut, "INIT", eq? "9009" : "6FF6");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_O);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_I0);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_I1);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 1));
	    edif_add_to_joint(jnt, lut, LUT_I2);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 1));
	    edif_add_to_joint(jnt, lut, LUT_I3);
	    return;

	  default:
	    { edif_cellref_t di;
	      di = edif_cellref_create(edf, eq? cell_0 : cell_1);
	      jnt_di = edif_joint_create(edf);
	      edif_add_to_joint(jnt_di, di, 0);
	    }

	    mux_prev = 0;
	    for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 2) {
		  int subwid = 2;
		  if ((idx + 1) == ivl_lpm_width(net))
			subwid = 1;

		  mux = edif_cellref_create(edf, xilinx_cell_muxcy(xlib));
		  if (subwid == 2) {
			lut = edif_cellref_create(edf, xilinx_cell_lut4(xlib));
			edif_cellref_pstring(lut, "INIT", "9009");
		  } else {
			lut = edif_cellref_create(edf, xilinx_cell_lut2(xlib));
			edif_cellref_pstring(lut, "INIT", "9");
		  }

		  jnt = edif_joint_create(edf);
		  edif_add_to_joint(jnt, lut, LUT_O);
		  edif_add_to_joint(jnt, mux, MUXCY_S);

		  jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
		  edif_add_to_joint(jnt, lut, LUT_I0);

		  jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, idx));
		  edif_add_to_joint(jnt, lut, LUT_I1);

		  if (subwid > 1) {
			jnt = edif_joint_of_nexus(edf,
					       ivl_lpm_data(net, idx+1));
			edif_add_to_joint(jnt, lut, LUT_I2);

			jnt = edif_joint_of_nexus(edf,
					       ivl_lpm_datab(net, idx+1));
			edif_add_to_joint(jnt, lut, LUT_I3);
		  }

		  edif_add_to_joint(jnt_di, mux, MUXCY_DI);

		  if (mux_prev) {
			jnt = edif_joint_create(edf);
			edif_add_to_joint(jnt, mux, MUXCY_CI);
			edif_add_to_joint(jnt, mux_prev, MUXCY_O);
		  } else {
			edif_cellref_t ci;
			ci = edif_cellref_create(edf, eq? cell_1 : cell_0);
			jnt = edif_joint_create(edf);
			edif_add_to_joint(jnt, ci, 0);
			edif_add_to_joint(jnt, mux, MUXCY_CI);
		  }

		  mux_prev = mux;
	    }

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
	    edif_add_to_joint(jnt, mux_prev, MUXCY_O);
	    return;
      }
}

/*
 * Implement hardware for the device (A >= B). We use LUT devices if
 * it can handle the slices, or carry chain logic if the slices must
 * span LUT devices.
 */
void virtex_ge(ivl_lpm_t net)
{
      edif_cellref_t muxcy_prev;
      edif_cellref_t lut;
      edif_joint_t jnt;
      unsigned idx;

      if (ivl_lpm_width(net) == 1) {

	      /* If the comparator is a single bit, then use a LUT2
		 with this truth table:

		 Q   A B
		 --+----
		 1 | 0 0
		 0 | 0 1
		 1 | 1 0
		 1 | 1 1

		 Connect the A value to I1 and the B value to I0. */

	    lut = edif_cellref_create(edf, xilinx_cell_lut2(xlib));
	    edif_cellref_pstring(lut, "INIT", "D");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_O);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_I1);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_I2);
	    return;
      }

	/* Handle the case where the device is two slices
	   wide. In this case, we can use a LUT4 to do all
	   the calculation. Use this truth table:

	      Q   AA BB
	      --+------
	      1 | 00 00
	      0 | 00 01
	      0 | 00 10
	      0 | 00 11
	      1 | 01 00
	      1 | 01 01
	      0 | 01 10
	      0 | 01 11
	      1 | 10 00
	      1 | 10 01
	      1 | 10 10
	      0 | 10 11
	      1 | 11 xx

	   The I3-I0 inputs are A1 A0 B1 B0 in that order. */

      assert(ivl_lpm_width(net) >= 2);

      lut = edif_cellref_create(edf, xilinx_cell_lut4(xlib));
      edif_cellref_pstring(lut, "INIT", "F731");

      jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 0));
      edif_add_to_joint(jnt, lut, LUT_I2);

      jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 0));
      edif_add_to_joint(jnt, lut, LUT_I0);

      jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 1));
      edif_add_to_joint(jnt, lut, LUT_I3);

      jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 1));
      edif_add_to_joint(jnt, lut, LUT_I1);

	/* There are only two slices, so this is all we need. */
      if (ivl_lpm_width(net) == 2) {
	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
	    edif_add_to_joint(jnt, lut, LUT_O);
	    return;
      }

	/* The general case requires that we make the >= comparator
	   from slices. This is an iterative design. Each slice has
	   the truth table:

	      An Bn | A >= B
	      ------+-------
	      0  0  |   CI
	      0  1  |   0
	      1  0  |   1
	      1  1  |   CI

	   The CI for each slice is the output of the compare of the
	   next less significant bits. We get this truth table by
	   connecting a LUT2 to the S input of a MUXCY. When the S
	   input is (1), it propagates its CI. This suggests that the
	   init value for the LUT be "9" (XNOR).

	   When the MUXCY S input is 0, it propagates a local
	   input. We connect to that input An, and we get the desired
	   and complete truth table for a slice.

	   This iterative definition needs to terminate at the least
	   significant bits. In fact, we have a non-iterative was to
	   deal with the two least significant slices. We take the
	   output of the LUT4 device for the least significant bits,
	   and use that to generate the initial CI for the chain. */


      muxcy_prev = edif_cellref_create(edf, xilinx_cell_muxcy_l(xlib));
      jnt = edif_joint_create(edf);

      edif_add_to_joint(jnt, lut, LUT_O);
      edif_add_to_joint(jnt, muxcy_prev, MUXCY_S);
      { edif_cellref_t p0 = edif_cellref_create(edf, cell_0);
        edif_cellref_t p1 = edif_cellref_create(edf, cell_1);

	jnt = edif_joint_create(edf);
	edif_add_to_joint(jnt, p0, 0);
	edif_add_to_joint(jnt, muxcy_prev, MUXCY_DI);

	jnt = edif_joint_create(edf);
	edif_add_to_joint(jnt, p1, 0);
	edif_add_to_joint(jnt, muxcy_prev, MUXCY_CI);
      }

      for (idx = 2 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    edif_cellref_t muxcy;

	    lut = edif_cellref_create(edf, xilinx_cell_lut2(xlib));
	    muxcy = edif_cellref_create(edf, xilinx_cell_muxcy(xlib));
	    edif_cellref_pstring(lut, "INIT", "9");

	    jnt = edif_joint_create(edf);
	    edif_add_to_joint(jnt, lut,   LUT_O);
	    edif_add_to_joint(jnt, muxcy, MUXCY_S);

	    jnt = edif_joint_create(edf);
	    edif_add_to_joint(jnt, muxcy, MUXCY_CI);
	    edif_add_to_joint(jnt, muxcy_prev, MUXCY_O);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
	    edif_add_to_joint(jnt, lut, LUT_I0);
	    edif_add_to_joint(jnt, muxcy, MUXCY_DI);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, idx));
	    edif_add_to_joint(jnt, lut, LUT_I1);

	    muxcy_prev = muxcy;
      }

      jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
      edif_add_to_joint(jnt, muxcy_prev, MUXCY_O);
}

/*
 * This function generates ADD/SUB devices for Virtex devices,
 * based on the documented implementations of ADD8/ADD16, etc., from
 * the Libraries Guide.
 *
 * Each slice of the ADD/SUB device is made from a LUT2 device, an
 * XORCY device that mixes with the LUT2 to make a full adder, and a
 * MUXCY_L to propagate the carry. The most significant slice does not
 * have a carry to propagate, so has no MUXCY_L.
 *
 * If the device is a wide adder, then the LUT2 devices are configured
 * to implement an XOR function and a zero is pumped into the least
 * significant carry input.
 *
 * If the device is really an adder, then the input is turned into an
 * XNOR, which takes a 1-s complement of the B input. Pump a 1 into
 * the LSB carry input to finish converting the B input into the 2s
 * complement.
 */
void virtex_add(ivl_lpm_t net)
{
      const char*ha_init = 0;
      edif_cellref_t lut, xorcy, muxcy, pad;
      edif_joint_t jnt;

      unsigned idx;

      if (ivl_lpm_width(net) < 2) {
	    xilinx_add(net);
	    return;
      }

      switch (ivl_lpm_type(net)) {
	    case IVL_LPM_ADD:
	    ha_init = "6";
	    break;
	  case IVL_LPM_SUB:
	    ha_init = "9";
	    break;
	  default:
	    assert(0);
      }

      assert(ivl_lpm_width(net) > 1);

      lut   = edif_cellref_create(edf, xilinx_cell_lut2(xlib));
      xorcy = edif_cellref_create(edf, xilinx_cell_xorcy(xlib));
      muxcy = edif_cellref_create(edf, xilinx_cell_muxcy_l(xlib));
      edif_cellref_pstring(lut, "INIT", ha_init);

	/* The bottom carry-in takes a constant that primes the add or
	   subtract. */
      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_ADD:
	    pad = edif_cellref_create(edf, cell_0);
	    break;

	  case IVL_LPM_SUB:
	    pad = edif_cellref_create(edf, cell_1);
	    break;

	  default:
	    assert(0);
      }

      jnt = edif_joint_create(edf);
      edif_add_to_joint(jnt, pad, 0);
      edif_add_to_joint(jnt, muxcy, MUXCY_CI);
      edif_add_to_joint(jnt, xorcy, XORCY_CI);

      jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
      edif_add_to_joint(jnt, xorcy, XORCY_O);

      jnt = edif_joint_create(edf);
      edif_add_to_joint(jnt, xorcy, XORCY_LI);
      edif_add_to_joint(jnt, muxcy, MUXCY_S);
      edif_add_to_joint(jnt, lut,   LUT_O);

      jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, 0));
      edif_add_to_joint(jnt, lut,   LUT_I0);
      edif_add_to_joint(jnt, muxcy, MUXCY_DI);

      jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, 0));
      edif_add_to_joint(jnt, lut, LUT_I1);

      for (idx = 1 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    edif_cellref_t muxcy0 = muxcy;

	    lut = edif_cellref_create(edf, xilinx_cell_lut2(xlib));
	    xorcy = edif_cellref_create(edf, xilinx_cell_xorcy(xlib));
	    edif_cellref_pstring(lut, "INIT", ha_init);

	      /* If this is the last bit, then there is no further
		 propagation in the carry chain, and I can skip the
		 carry mux MUXCY. */
	    if ((idx+1) < ivl_lpm_width(net))
		  muxcy = edif_cellref_create(edf, xilinx_cell_muxcy_l(xlib));
	    else
		  muxcy = 0;

	    jnt = edif_joint_create(edf);
	    edif_add_to_joint(jnt, muxcy0, MUXCY_O);
	    edif_add_to_joint(jnt, xorcy, XORCY_CI);
	    if (muxcy) edif_add_to_joint(jnt, muxcy, MUXCY_CI);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, idx));
	    edif_add_to_joint(jnt, xorcy, XORCY_O);

	    jnt = edif_joint_create(edf);
	    edif_add_to_joint(jnt, xorcy, XORCY_LI);
	    if (muxcy) edif_add_to_joint(jnt, muxcy, MUXCY_S);
	    edif_add_to_joint(jnt, lut,   LUT_O);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
	    edif_add_to_joint(jnt, lut,   LUT_I0);
	    if (muxcy) edif_add_to_joint(jnt, muxcy, MUXCY_DI);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, idx));
	    edif_add_to_joint(jnt, lut, LUT_I1);
      }

}


const struct device_s d_virtex_edif = {
      virtex_show_header,
      virtex_show_footer,
      xilinx_show_scope,
      xilinx_pad,
      xilinx_logic,
      virtex_generic_dff,
      virtex_eq,
      virtex_eq,
      virtex_ge,
      0,
      virtex_add,
      virtex_add,
      xilinx_shiftl,
      0  /* show_shiftr */
};


/*
 * $Log: d-virtex.c,v $
 * Revision 1.25  2003/06/25 02:55:57  steve
 *  Virtex and Virtex2 share much code.
 *
 * Revision 1.24  2003/06/25 01:49:06  steve
 *  Spelling fixes.
 *
 * Revision 1.23  2003/06/24 03:55:00  steve
 *  Add ivl_synthesis_cell support for virtex2.
 *
 * Revision 1.22  2003/02/26 01:24:42  steve
 *  ivl_lpm_name is obsolete.
 */

