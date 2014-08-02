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
# include  <assert.h>

/*
 * This is the device emitter for the most generic FPGA. It doesn't
 * know anything special about device types, so can't handle complex
 * logic.
 */

static void xnf_draw_pin(ivl_nexus_t nex, const char*nam, char dir)
{
      const char*use_name = nam;
      const char*nex_name = xnf_mangle_nexus_name(nex);
      int invert = 0;

      if (use_name[0] == '~') {
	    invert = 1;
	    use_name += 1;
      }

      fprintf(xnf, "    PIN, %s, %c, %s", use_name, dir, nex_name);

      if (invert)
	    fprintf(xnf, ",,INV");

      fprintf(xnf, "\n");
}

static void show_root_ports_xnf(ivl_scope_t root)
{
      unsigned cnt = ivl_scope_sigs(root);
      unsigned idx;

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);
	    const char*use_name;

	    if (ivl_signal_port(sig) == IVL_SIP_NONE)
		  continue;

	    use_name = ivl_signal_basename(sig);
	    if (ivl_signal_pins(sig) == 1) {
		  ivl_nexus_t nex = ivl_signal_pin(sig, 0);
		  fprintf(xnf, "SIG, %s, PIN=%s\n",
			  xnf_mangle_nexus_name(nex), use_name);

	    } else {
		  unsigned pin;

		  for (pin = 0 ; pin < ivl_signal_pins(sig); pin += 1) {
			ivl_nexus_t nex = ivl_signal_pin(sig, pin);
			fprintf(xnf, "SIG, %s, PIN=%s%u\n",
				xnf_mangle_nexus_name(nex), use_name,
				pin);
		  }
	    }
      }
}

static void show_design_consts_xnf(ivl_design_t des)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_design_consts(des) ;  idx += 1) {
	    unsigned pin;
	    ivl_net_const_t net = ivl_design_const(des, idx);
	    const char*val = ivl_const_bits(net);

	    for (pin = 0 ;  pin < ivl_const_pins(net) ;  pin += 1) {
		  ivl_nexus_t nex = ivl_const_pin(net, pin);
		  fprintf(xnf, "PWR,%c,%s\n", val[pin],
			  xnf_mangle_nexus_name(nex));
	    }
      }
}

static void generic_show_header(ivl_design_t des)
{
      ivl_scope_t root = ivl_design_root(des);

      fprintf(xnf, "LCANET,6\n");
      fprintf(xnf, "PROG,iverilog,$Name:  $,\"Icarus Verilog/fpga.tgt\"\n");

      if (part && (part[0]!=0)) {
	    fprintf(xnf, "PART,%s\n", part);
      }

      show_root_ports_xnf(root);
}

static void generic_show_footer(ivl_design_t des)
{
      show_design_consts_xnf(des);
      fprintf(xnf, "EOF\n");
}


static void generic_show_logic(ivl_net_logic_t net)
{
      char name[1024];
      ivl_nexus_t nex;
      unsigned idx;

      xnf_mangle_logic_name(net, name, sizeof name);

      switch (ivl_logic_type(net)) {

	  case IVL_LO_AND:
	    fprintf(xnf, "SYM, %s, AND, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
		  char ipin[32];
		  nex = ivl_logic_pin(net, idx);
		  sprintf(ipin, "I%u", idx-1);
		  xnf_draw_pin(nex, ipin, 'I');
	    }
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_BUF:
	    assert(ivl_logic_pins(net) == 2);
	    fprintf(xnf, "SYM, %s, BUF, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    nex = ivl_logic_pin(net, 1);
	    xnf_draw_pin(nex, "I", 'I');
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_NAND:
	    fprintf(xnf, "SYM, %s, NAND, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
		  char ipin[32];
		  nex = ivl_logic_pin(net, idx);
		  sprintf(ipin, "I%u", idx-1);
		  xnf_draw_pin(nex, ipin, 'I');
	    }
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_NOR:
	    fprintf(xnf, "SYM, %s, NOR, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
		  char ipin[32];
		  nex = ivl_logic_pin(net, idx);
		  sprintf(ipin, "I%u", idx-1);
		  xnf_draw_pin(nex, ipin, 'I');
	    }
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_NOT:
	    assert(ivl_logic_pins(net) == 2);
	    fprintf(xnf, "SYM, %s, INV, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    nex = ivl_logic_pin(net, 1);
	    xnf_draw_pin(nex, "I", 'I');
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_OR:
	    fprintf(xnf, "SYM, %s, OR, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
		  char ipin[32];
		  nex = ivl_logic_pin(net, idx);
		  sprintf(ipin, "I%u", idx-1);
		  xnf_draw_pin(nex, ipin, 'I');
	    }
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_XOR:
	    fprintf(xnf, "SYM, %s, XOR, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
		  char ipin[32];
		  nex = ivl_logic_pin(net, idx);
		  sprintf(ipin, "I%u", idx-1);
		  xnf_draw_pin(nex, ipin, 'I');
	    }
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_XNOR:
	    fprintf(xnf, "SYM, %s, XNOR, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
		  char ipin[32];
		  nex = ivl_logic_pin(net, idx);
		  sprintf(ipin, "I%u", idx-1);
		  xnf_draw_pin(nex, ipin, 'I');
	    }
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_BUFIF0:
	    fprintf(xnf, "SYM, %s, TBUF, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    nex = ivl_logic_pin(net, 1);
	    xnf_draw_pin(nex, "I", 'I');
	    nex = ivl_logic_pin(net, 2);
	    xnf_draw_pin(nex, "~T", 'I');
	    fprintf(xnf, "END\n");
	    break;

	  case IVL_LO_BUFIF1:
	    fprintf(xnf, "SYM, %s, TBUF, LIBVER=2.0.0\n", name);
	    nex = ivl_logic_pin(net, 0);
	    xnf_draw_pin(nex, "O", 'O');
	    nex = ivl_logic_pin(net, 1);
	    xnf_draw_pin(nex, "I", 'I');
	    nex = ivl_logic_pin(net, 2);
	    xnf_draw_pin(nex, "T", 'I');
	    fprintf(xnf, "END\n");
	    break;

	  default:
	    fprintf(stderr, "fpga.tgt: unknown logic type %d\n",
		    ivl_logic_type(net));
	    break;
      }

}

static void generic_show_dff(ivl_lpm_t net)
{
      char name[1024];
      ivl_nexus_t nex;

      xnf_mangle_lpm_name(net, name, sizeof name);

      fprintf(xnf, "SYM, %s, DFF, LIBVER=2.0.0\n", name);

      nex = ivl_lpm_q(net, 0);
      xnf_draw_pin(nex, "Q", 'O');

      nex = ivl_lpm_data(net, 0);
      xnf_draw_pin(nex, "D", 'I');

      nex = ivl_lpm_clk(net);
      xnf_draw_pin(nex, "C", 'I');

      if ((nex = ivl_lpm_enable(net)))
	    xnf_draw_pin(nex, "CE", 'I');

      fprintf(xnf, "END\n");
}

/*
 * The generic == comparator uses EQN records to generate 2-bit
 * comparators, that are then connected together by a wide AND gate.
 */
static void generic_show_cmp_eq(ivl_lpm_t net)
{
      ivl_nexus_t nex;
      unsigned idx;
      char name[1024];
	/* Make this many dual pair comparators, and */
      unsigned deqn = ivl_lpm_width(net) / 2;
	/* Make this many single pair comparators. */
      unsigned seqn = ivl_lpm_width(net) % 2;

      xnf_mangle_lpm_name(net, name, sizeof name);

      for (idx = 0 ;  idx < deqn ;  idx += 1) {
	    fprintf(xnf, "SYM, %s/CD%u, EQN, "
		    "EQN=(~((I0 @ I1) + (I2 @ I3)))\n",
		    name, idx);

	    fprintf(xnf, "    PIN, O, O, %s/CDO%u\n", name, idx);

	    nex = ivl_lpm_data(net, 2*idx);
	    xnf_draw_pin(nex, "I0", 'I');
	    nex = ivl_lpm_datab(net, 2*idx);
	    xnf_draw_pin(nex, "I1", 'I');

	    nex = ivl_lpm_data(net, 2*idx+1);
	    xnf_draw_pin(nex, "I2", 'I');
	    nex = ivl_lpm_datab(net, 2*idx+1);
	    xnf_draw_pin(nex, "I3", 'I');

	    fprintf(xnf, "END\n");
      }

      if (seqn != 0) {
	    fprintf(xnf, "SYM, %s/CT, XNOR, LIBVER=2.0.0\n", name);

	    fprintf(xnf, "    PIN, O, O, %s/CTO\n", name);

	    nex = ivl_lpm_data(net, 2*deqn);
	    xnf_draw_pin(nex, "I0", 'I');

	    nex = ivl_lpm_datab(net, 2*deqn);
	    xnf_draw_pin(nex, "I1", 'I');

	    fprintf(xnf, "END\n");
      }

      if (ivl_lpm_type(net) == IVL_LPM_CMP_EQ)
	    fprintf(xnf, "SYM, %s/OUT, AND, LIBVER=2.0.0\n", name);
      else
	    fprintf(xnf, "SYM, %s/OUT, NAND, LIBVER=2.0.0\n", name);

      nex = ivl_lpm_q(net, 0);
      xnf_draw_pin(nex, "O", 'O');

      for (idx = 0 ;  idx < deqn ;  idx += 1)
	    fprintf(xnf, "    PIN, I%u, I, %s/CDO%u\n", idx, name, idx);

      for (idx = 0 ;  idx < seqn ;  idx += 1)
	    fprintf(xnf, "    PIN, I%u, I, %s/CTO\n", deqn+idx, name);

      fprintf(xnf, "END\n");
}

/*
 * This function draws N-bit wide binary mux devices. These are so
 * very popular because they are the result of such expressions as:
 *
 *        x = sel? a : b;
 *
 * This code only supports the case where sel is a single bit. It
 * works by drawing for each bit of the width an EQN device that takes
 * as inputs I0 and I1 the alternative inputs, and I2 the select. The
 * select bit is common with all the generated mux devices.
 */
static void generic_show_mux(ivl_lpm_t net)
{
      char name[1024];
      ivl_nexus_t sel;
      unsigned idx;

      xnf_mangle_lpm_name(net, name, sizeof name);

	/* Access the single select bit. This is common to the whole
	   width of the mux. */
      assert(ivl_lpm_selects(net) == 1);
      sel = ivl_lpm_select(net, 0);

      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    ivl_nexus_t nex;

	    fprintf(xnf, "SYM, %s/M%u, EQN, "
		    "EQN=((I0 * ~I2) + (I1 * I2))\n",
		    name, idx);

	    nex = ivl_lpm_q(net, idx);
	    xnf_draw_pin(nex, "O", 'O');

	    nex = ivl_lpm_data2(net, 0, idx);
	    xnf_draw_pin(nex, "I0", 'I');

	    nex = ivl_lpm_data2(net, 1, idx);
	    xnf_draw_pin(nex, "I1", 'I');

	    xnf_draw_pin(sel, "I2", 'I');

	    fprintf(xnf, "END\n");
      }
}

/*
 * This code cheats and just generates ADD4 devices enough to support
 * the add. Make no effort to optimize, because we have no idea what
 * kind of device we have.
 */
static void generic_show_add(ivl_lpm_t net)
{
      char name[1024];
      ivl_nexus_t nex;
      unsigned idx, nadd4, tail;

      xnf_mangle_lpm_name(net, name, sizeof name);

	/* Make this many ADD4 devices. */
      nadd4 = ivl_lpm_width(net) / 4;
      tail  = ivl_lpm_width(net) % 4;

      for (idx = 0 ;  idx < nadd4 ;  idx += 1) {
	    fprintf(xnf, "SYM, %s/A%u, ADD4\n", name, idx);

	    if (idx > 0)
		  fprintf(xnf, "    PIN, CI, I, %s/CO%u\n", name, idx-1);

	    nex = ivl_lpm_q(net, idx*4+0);
	    xnf_draw_pin(nex, "S0", 'O');

	    nex = ivl_lpm_q(net, idx*4+1);
	    xnf_draw_pin(nex, "S1", 'O');

	    nex = ivl_lpm_q(net, idx*4+2);
	    xnf_draw_pin(nex, "S2", 'O');

	    nex = ivl_lpm_q(net, idx*4+3);
	    xnf_draw_pin(nex, "S3", 'O');

	    nex = ivl_lpm_data(net, idx*4+0);
	    xnf_draw_pin(nex, "A0", 'I');

	    nex = ivl_lpm_data(net, idx*4+1);
	    xnf_draw_pin(nex, "A1", 'I');

	    nex = ivl_lpm_data(net, idx*4+2);
	    xnf_draw_pin(nex, "A2", 'I');

	    nex = ivl_lpm_data(net, idx*4+3);
	    xnf_draw_pin(nex, "A3", 'I');

	    nex = ivl_lpm_datab(net, idx*4+0);
	    xnf_draw_pin(nex, "B0", 'I');

	    nex = ivl_lpm_datab(net, idx*4+1);
	    xnf_draw_pin(nex, "B1", 'I');

	    nex = ivl_lpm_datab(net, idx*4+2);
	    xnf_draw_pin(nex, "B2", 'I');

	    nex = ivl_lpm_datab(net, idx*4+3);
	    xnf_draw_pin(nex, "B3", 'I');

	    if ((idx*4+4) < ivl_lpm_width(net))
		  fprintf(xnf, "    PIN, CO, O, %s/CO%u\n", name, idx);

	    fprintf(xnf, "END\n");
      }

      if (tail > 0) {
	    fprintf(xnf, "SYM, %s/A%u, ADD4\n", name, nadd4);
	    if (nadd4 > 0)
		  fprintf(xnf, "    PIN, CI, I, %s/CO%u\n", name, nadd4-1);

	    switch (tail) {
		case 3:
		  nex = ivl_lpm_data(net, nadd4*4+2);
		  xnf_draw_pin(nex, "A2", 'I');

		  nex = ivl_lpm_datab(net, nadd4*4+2);
		  xnf_draw_pin(nex, "B2", 'I');

		  nex = ivl_lpm_q(net, nadd4*4+2);
		  xnf_draw_pin(nex, "S2", 'O');
		case 2:
		  nex = ivl_lpm_data(net, nadd4*4+1);
		  xnf_draw_pin(nex, "A1", 'I');

		  nex = ivl_lpm_datab(net, nadd4*4+1);
		  xnf_draw_pin(nex, "B1", 'I');

		  nex = ivl_lpm_q(net, nadd4*4+1);
		  xnf_draw_pin(nex, "S1", 'O');
		case 1:
		  nex = ivl_lpm_data(net, nadd4*4+0);
		  xnf_draw_pin(nex, "A0", 'I');

		  nex = ivl_lpm_datab(net, nadd4*4+0);
		  xnf_draw_pin(nex, "B0", 'I');

		  nex = ivl_lpm_q(net, nadd4*4+0);
		  xnf_draw_pin(nex, "S0", 'O');
	    }

	    fprintf(xnf, "END\n");
      }
}

const struct device_s d_generic = {
      generic_show_header,
      generic_show_footer,
      0, /* show_scope */
      0, /* show_pad not implemented */
      generic_show_logic,
      generic_show_dff,
      generic_show_cmp_eq,
      generic_show_cmp_eq,
      0, /* ge not implemented */
      0, /* gt not implemented */
      generic_show_mux,
      generic_show_add,
      0, /* subtract not implemented */
      0,
      0
};
