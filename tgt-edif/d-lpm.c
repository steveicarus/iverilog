/*
 * Copyright (c) 2005-2010 Stephen Williams
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is the driver for a purely generic LPM module writer. This
 * uses LPM version 2 1 0 devices, without particularly considering
 * the target technology.
 *
 * The LPM standard is EIA-IS/103-A  October 1996
 * The output is EDIF 2 0 0 format.
 */

# include  "device.h"
# include  "edif_priv.h"
# include  "edif.h"
# include  "generic.h"
# include  <string.h>
# include  <assert.h>

static edif_cell_t lpm_cell_buf(void)
{
      static edif_cell_t tmp = 0;

      if (tmp != 0)
	    return tmp;

      tmp = edif_xcell_create(xlib, "BUF", 2);
      edif_cell_portconfig(tmp, 0, "Result", IVL_SIP_OUTPUT);
      edif_cell_portconfig(tmp, 1, "Data",   IVL_SIP_INPUT);

	/* A buffer is an inverted inverter. */
      edif_cell_port_pstring(tmp, 0, "LPM_Polarity", "INVERT");

      edif_cell_pstring(tmp,  "LPM_TYPE",  "LPM_INV");
      edif_cell_pinteger(tmp, "LPM_Width", 1);
      edif_cell_pinteger(tmp, "LPM_Size",  1);
      return tmp;
}

static edif_cell_t lpm_cell_inv(void)
{
      static edif_cell_t tmp = 0;

      if (tmp != 0)
	    return tmp;

      tmp = edif_xcell_create(xlib, "INV", 2);
      edif_cell_portconfig(tmp, 0, "Result", IVL_SIP_OUTPUT);
      edif_cell_portconfig(tmp, 1, "Data",   IVL_SIP_INPUT);

      edif_cell_pstring(tmp,  "LPM_TYPE",  "LPM_INV");
      edif_cell_pinteger(tmp, "LPM_Width", 1);
      edif_cell_pinteger(tmp, "LPM_Size",  1);
      return tmp;
}

static edif_cell_t lpm_cell_bufif0(void)
{
      static edif_cell_t tmp = 0;

      if (tmp != 0)
	    return tmp;

      tmp = edif_xcell_create(xlib, "BUFIF1", 3);
      edif_cell_portconfig(tmp, 0, "TriData",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(tmp, 1, "Data",     IVL_SIP_INPUT);
      edif_cell_portconfig(tmp, 2, "EnableDT", IVL_SIP_INPUT);

      edif_cell_port_pstring(tmp, 2, "LPM_Polarity", "INVERT");

      edif_cell_pstring(tmp,  "LPM_TYPE",  "LPM_BUSTRI");
      edif_cell_pinteger(tmp, "LPM_Width", 1);
      return tmp;
}

static edif_cell_t lpm_cell_bufif1(void)
{
      static edif_cell_t tmp = 0;

      if (tmp != 0)
	    return tmp;

      tmp = edif_xcell_create(xlib, "BUFIF1", 3);
      edif_cell_portconfig(tmp, 0, "TriData",  IVL_SIP_OUTPUT);
      edif_cell_portconfig(tmp, 1, "Data",     IVL_SIP_INPUT);
      edif_cell_portconfig(tmp, 2, "EnableDT", IVL_SIP_INPUT);

      edif_cell_pstring(tmp,  "LPM_TYPE",  "LPM_BUSTRI");
      edif_cell_pinteger(tmp, "LPM_Width", 1);
      return tmp;
}

static edif_cell_t lpm_cell_or(unsigned siz)
{
      unsigned idx;
      edif_cell_t cell;
      char name[32];

      sprintf(name, "or%u", siz);

      cell = edif_xlibrary_findcell(xlib, name);
      if (cell != 0)
	    return cell;

      cell = edif_xcell_create(xlib, strdup(name), siz+1);

      edif_cell_portconfig(cell, 0, "Result0", IVL_SIP_OUTPUT);

      for (idx = 0 ;  idx < siz ;  idx += 1) {
	    sprintf(name, "Data%ux0", idx);
	    edif_cell_portconfig(cell, idx+1, strdup(name), IVL_SIP_INPUT);
      }

      edif_cell_pstring(cell,  "LPM_TYPE",  "LPM_OR");
      edif_cell_pinteger(cell, "LPM_Width", 1);
      edif_cell_pinteger(cell, "LPM_Size",  siz);

      return cell;
}

static edif_cell_t lpm_cell_and(unsigned siz)
{
      unsigned idx;
      edif_cell_t cell;
      char name[32];

      sprintf(name, "and%u", siz);

      cell = edif_xlibrary_findcell(xlib, name);
      if (cell != 0)
	    return cell;

      cell = edif_xcell_create(xlib, strdup(name), siz+1);

      edif_cell_portconfig(cell, 0, "Result0", IVL_SIP_OUTPUT);

      for (idx = 0 ;  idx < siz ;  idx += 1) {
	    sprintf(name, "Data%ux0", idx);
	    edif_cell_portconfig(cell, idx+1, strdup(name), IVL_SIP_INPUT);
      }

      edif_cell_pstring(cell,  "LPM_TYPE",  "LPM_AND");
      edif_cell_pinteger(cell, "LPM_Width", 1);
      edif_cell_pinteger(cell, "LPM_Size",  siz);

      return cell;
}

static edif_cell_t lpm_cell_xor(unsigned siz)
{
      unsigned idx;
      edif_cell_t cell;
      char name[32];

      sprintf(name, "xor%u", siz);

      cell = edif_xlibrary_findcell(xlib, name);
      if (cell != 0)
	    return cell;

      cell = edif_xcell_create(xlib, strdup(name), siz+1);

      edif_cell_portconfig(cell, 0, "Result0", IVL_SIP_OUTPUT);

      for (idx = 0 ;  idx < siz ;  idx += 1) {
	    sprintf(name, "Data%ux0", idx);
	    edif_cell_portconfig(cell, idx+1, strdup(name), IVL_SIP_INPUT);
      }

      edif_cell_pstring(cell,  "LPM_TYPE",  "LPM_XOR");
      edif_cell_pinteger(cell, "LPM_Width", 1);
      edif_cell_pinteger(cell, "LPM_Size",  siz);

      return cell;
}

static edif_cell_t lpm_cell_nor(unsigned siz)
{
      unsigned idx;
      edif_cell_t cell;
      char name[32];

      sprintf(name, "nor%u", siz);

      cell = edif_xlibrary_findcell(xlib, name);
      if (cell != 0)
	    return cell;

      cell = edif_xcell_create(xlib, strdup(name), siz+1);

      edif_cell_portconfig(cell, 0, "Result0", IVL_SIP_OUTPUT);
      edif_cell_port_pstring(cell, 0, "LPM_Polarity", "INVERT");

      for (idx = 0 ;  idx < siz ;  idx += 1) {
	    sprintf(name, "Data%ux0", idx);
	    edif_cell_portconfig(cell, idx+1, strdup(name), IVL_SIP_INPUT);
      }

      edif_cell_pstring(cell,  "LPM_TYPE",  "LPM_OR");
      edif_cell_pinteger(cell, "LPM_Width", 1);
      edif_cell_pinteger(cell, "LPM_Size",  siz);

      return cell;
}

static edif_cell_t lpm_cell_nand(unsigned siz)
{
      unsigned idx;
      edif_cell_t cell;
      char name[32];

      sprintf(name, "nand%u", siz);

      cell = edif_xlibrary_findcell(xlib, name);
      if (cell != 0)
	    return cell;

      cell = edif_xcell_create(xlib, strdup(name), siz+1);

      edif_cell_portconfig(cell, 0, "Result0", IVL_SIP_OUTPUT);
      edif_cell_port_pstring(cell, 0, "LPM_Polarity", "INVERT");

      for (idx = 0 ;  idx < siz ;  idx += 1) {
	    sprintf(name, "Data%ux0", idx);
	    edif_cell_portconfig(cell, idx+1, strdup(name), IVL_SIP_INPUT);
      }

      edif_cell_pstring(cell,  "LPM_TYPE",  "LPM_AND");
      edif_cell_pinteger(cell, "LPM_Width", 1);
      edif_cell_pinteger(cell, "LPM_Size",  siz);

      return cell;
}

static void lpm_show_header(ivl_design_t des)
{
      unsigned idx;
      ivl_scope_t root = ivl_design_root(des);
      unsigned sig_cnt = ivl_scope_sigs(root);
      unsigned nports = 0, pidx;

	/* Count the ports I'm going to use. */
      for (idx = 0 ;  idx < sig_cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);

	    if (ivl_signal_port(sig) == IVL_SIP_NONE)
		  continue;

	    if (ivl_signal_attr(sig, "PAD") != 0)
		  continue;

	    nports += ivl_signal_pins(sig);
      }

	/* Create the base edf object. */
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

      xlib = edif_xlibrary_create(edf, "LPM_LIBRARY");
}

static void lpm_show_footer(ivl_design_t des)
{
      edif_print(xnf, edf);
}

static void hookup_logic_gate(ivl_net_logic_t net, edif_cell_t cell)
{
      unsigned pin, idx;

      edif_joint_t jnt;
      edif_cellref_t ref = edif_cellref_create(edf, cell);

      jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 0));
      pin = edif_cell_port_byname(cell, "Result0");
      edif_add_to_joint(jnt, ref, pin);

      for (idx = 1 ;  idx < ivl_logic_pins(net) ;  idx += 1) {
	    char name[32];

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, idx));
	    sprintf(name, "Data%ux0", idx-1);
	    pin = edif_cell_port_byname(cell, name);
	    edif_add_to_joint(jnt, ref, pin);
      }
}

static void lpm_logic(ivl_net_logic_t net)
{
      edif_cell_t cell;
      edif_cellref_t ref;
      edif_joint_t jnt;

      switch (ivl_logic_type(net)) {

	  case IVL_LO_BUFZ:
	  case IVL_LO_BUF:
	    assert(ivl_logic_pins(net) == 2);
	    cell = lpm_cell_buf();
	    ref = edif_cellref_create(edf, cell);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 0));
	    edif_add_to_joint(jnt, ref, 0);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 1));
	    edif_add_to_joint(jnt, ref, 1);
	    break;

	  case IVL_LO_BUFIF0:
	    assert(ivl_logic_pins(net) == 3);
	    cell = lpm_cell_bufif0();
	    ref = edif_cellref_create(edf, cell);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 0));
	    edif_add_to_joint(jnt, ref, 0);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 1));
	    edif_add_to_joint(jnt, ref, 1);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 2));
	    edif_add_to_joint(jnt, ref, 2);
	    break;

	  case IVL_LO_BUFIF1:
	    assert(ivl_logic_pins(net) == 3);
	    cell = lpm_cell_bufif1();
	    ref = edif_cellref_create(edf, cell);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 0));
	    edif_add_to_joint(jnt, ref, 0);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 1));
	    edif_add_to_joint(jnt, ref, 1);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 2));
	    edif_add_to_joint(jnt, ref, 2);
	    break;

	  case IVL_LO_NOT:
	    assert(ivl_logic_pins(net) == 2);
	    cell = lpm_cell_inv();
	    ref = edif_cellref_create(edf, cell);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 0));
	    edif_add_to_joint(jnt, ref, 0);

	    jnt = edif_joint_of_nexus(edf, ivl_logic_pin(net, 1));
	    edif_add_to_joint(jnt, ref, 1);
	    break;

	  case IVL_LO_OR:
	    cell = lpm_cell_or(ivl_logic_pins(net)-1);
	    hookup_logic_gate(net, cell);
	    break;

	  case IVL_LO_NOR:
	    cell = lpm_cell_nor(ivl_logic_pins(net)-1);
	    hookup_logic_gate(net, cell);
	    break;

	  case IVL_LO_AND:
	    cell = lpm_cell_and(ivl_logic_pins(net)-1);
	    hookup_logic_gate( net, cell);
	    break;

	  case IVL_LO_NAND:
	    cell = lpm_cell_nand(ivl_logic_pins(net)-1);
	    hookup_logic_gate( net, cell);
	    break;

	  case IVL_LO_XOR:
	    cell = lpm_cell_xor(ivl_logic_pins(net)-1);
	    hookup_logic_gate( net, cell);
	    break;

	  default:
	    fprintf(stderr, "UNSUPPORTED LOGIC TYPE: %u\n",
		    ivl_logic_type(net));
	    break;
      }
}


static void lpm_show_dff(ivl_lpm_t net)
{
      char name[64];
      edif_cell_t cell;
      edif_cellref_t ref;
      edif_joint_t jnt;

      unsigned idx;
      unsigned pin, wid = ivl_lpm_width(net);

      sprintf(name, "fd%s%s%s%s%s%u",
	      ivl_lpm_enable(net)? "ce" : "",
	      ivl_lpm_async_clr(net)? "cl" : "",
	      ivl_lpm_sync_clr(net)? "sc" : "",
	      ivl_lpm_async_set(net)? "se" : "",
	      ivl_lpm_sync_set(net)? "ss" : "",
	      wid);

      cell = edif_xlibrary_findcell(xlib, name);

      if (cell == 0) {
	    unsigned nports = 2 * wid + 1;
	    pin = 0;
	    if (ivl_lpm_enable(net))
		  nports += 1;
	    if (ivl_lpm_async_clr(net))
		  nports += 1;
	    if (ivl_lpm_sync_clr(net))
		  nports += 1;
	    if (ivl_lpm_async_set(net))
		  nports += 1;
	    if (ivl_lpm_sync_set(net))
		  nports += 1;

	    cell = edif_xcell_create(xlib, strdup(name), nports);
	    edif_cell_pstring(cell,  "LPM_Type", "LPM_FF");
	    edif_cell_pinteger(cell, "LPM_Width", wid);

	    for (idx = 0 ;  idx < wid ;  idx += 1) {

		  sprintf(name, "Q%u", idx);
		  edif_cell_portconfig(cell, idx*2+0, strdup(name),
				       IVL_SIP_OUTPUT);

		  sprintf(name, "Data%u", idx);
		  edif_cell_portconfig(cell, idx*2+1, strdup(name),
				       IVL_SIP_INPUT);
	    }

	    pin = wid*2;

	    if (ivl_lpm_enable(net)) {
		  edif_cell_portconfig(cell, pin, "Enable", IVL_SIP_INPUT);
		  pin += 1;
	    }

	    if (ivl_lpm_async_clr(net)) {
		  edif_cell_portconfig(cell, pin, "Aclr", IVL_SIP_INPUT);
		  pin += 1;
	    }

	    if (ivl_lpm_sync_clr(net)) {
		  edif_cell_portconfig(cell, pin, "Sclr", IVL_SIP_INPUT);
		  pin += 1;
	    }

	    if (ivl_lpm_async_set(net)) {
		  edif_cell_portconfig(cell, pin, "Aset", IVL_SIP_INPUT);
		  pin += 1;
	    }

	    if (ivl_lpm_sync_set(net)) {
		  edif_cell_portconfig(cell, pin, "Sset", IVL_SIP_INPUT);
		  pin += 1;
	    }

	    edif_cell_portconfig(cell, pin, "Clock", IVL_SIP_INPUT);
	    pin += 1;

	    assert(pin == nports);
      }

      ref = edif_cellref_create(edf, cell);

      pin = edif_cell_port_byname(cell, "Clock");

      jnt = edif_joint_of_nexus(edf, ivl_lpm_clk(net));
      edif_add_to_joint(jnt, ref, pin);

      if (ivl_lpm_enable(net)) {
	    pin = edif_cell_port_byname(cell, "Enable");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_enable(net));
	    edif_add_to_joint(jnt, ref, pin);
      }

      if (ivl_lpm_async_clr(net)) {
	    pin = edif_cell_port_byname(cell, "Aclr");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_async_clr(net));
	    edif_add_to_joint(jnt, ref, pin);
      }

      if (ivl_lpm_sync_clr(net)) {
	    pin = edif_cell_port_byname(cell, "Sclr");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_sync_clr(net));
	    edif_add_to_joint(jnt, ref, pin);
      }

      if (ivl_lpm_async_set(net)) {
	    pin = edif_cell_port_byname(cell, "Aset");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_async_set(net));
	    edif_add_to_joint(jnt, ref, pin);
      }

      if (ivl_lpm_sync_set(net)) {
	    ivl_expr_t svalue = ivl_lpm_sset_value(net);

	    pin = edif_cell_port_byname(cell, "Sset");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_sync_set(net));
	    edif_add_to_joint(jnt, ref, pin);

	    edif_cellref_pinteger(ref, "LPM_Svalue", ivl_expr_uvalue(svalue));
      }

      for (idx = 0 ;  idx < wid ;  idx += 1) {

	    sprintf(name, "Q%u", idx);
	    pin = edif_cell_port_byname(cell, name);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, idx));
	    edif_add_to_joint(jnt, ref, pin);

	    sprintf(name, "Data%u", idx);
	    pin = edif_cell_port_byname(cell, name);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
	    edif_add_to_joint(jnt, ref, pin);
      }
}

static void lpm_show_cmp_eq(ivl_lpm_t net)
{
      edif_cell_t cell;
      edif_cellref_t ref;
      edif_joint_t jnt;
      unsigned idx;

      char cellname[32];

      unsigned width = ivl_lpm_width(net);

      sprintf(cellname, "compare%u_eq", width);
      cell = edif_xlibrary_findcell(xlib, cellname);

      if (cell == 0) {
	    unsigned pins = 2*width + 1;

	    cell = edif_xcell_create(xlib, strdup(cellname), pins);

	      /* Make the output port. */
	    sprintf(cellname, "AEB");
	    edif_cell_portconfig(cell, 2*width, strdup(cellname),
				 IVL_SIP_OUTPUT);

	    for (idx = 0 ;  idx < width ;  idx += 1) {
		  sprintf(cellname, "DataA%u", idx);
		  edif_cell_portconfig(cell, idx+0, strdup(cellname),
				       IVL_SIP_INPUT);
	    }

	    for (idx = 0 ;  idx < width ;  idx += 1) {
		  sprintf(cellname, "DataB%u", idx);
		  edif_cell_portconfig(cell, idx+width, strdup(cellname),
				       IVL_SIP_INPUT);
	    }

	    edif_cell_pstring(cell,  "LPM_Type",   "LPM_COMPARE");
	    edif_cell_pinteger(cell, "LPM_Width",  width);
      }

      ref = edif_cellref_create(edf, cell);

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    unsigned pin;

	    sprintf(cellname, "DataA%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
	    edif_add_to_joint(jnt, ref, pin);
      }

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    unsigned pin;

	    sprintf(cellname, "DataB%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, idx));
	    edif_add_to_joint(jnt, ref, pin);
      }

      {
	    unsigned pin;

	    pin = edif_cell_port_byname(cell, "AEB");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, 0));
	    edif_add_to_joint(jnt, ref, pin);
      }
}

static void lpm_show_mux(ivl_lpm_t net)
{
      edif_cell_t cell;
      edif_cellref_t ref;
      edif_joint_t jnt;

      unsigned idx, rdx;

      char cellname[32];

      unsigned wid_r = ivl_lpm_width(net);
      unsigned wid_s = ivl_lpm_selects(net);
      unsigned wid_z = ivl_lpm_size(net);

      sprintf(cellname, "mux%u_%u_%u", wid_r, wid_s, wid_z);
      cell = edif_xlibrary_findcell(xlib, cellname);

      if (cell == 0) {
	    unsigned pins = wid_r + wid_s + wid_r*wid_z;

	    cell = edif_xcell_create(xlib, strdup(cellname), pins);

	      /* Make the output ports. */
	    for (idx = 0 ;  idx < wid_r ;  idx += 1) {
		  sprintf(cellname, "Result%u", idx);
		  edif_cell_portconfig(cell, idx, strdup(cellname),
				       IVL_SIP_OUTPUT);
	    }

	      /* Make the select ports. */
	    for (idx = 0 ;  idx < wid_s ;  idx += 1) {
		  sprintf(cellname, "Sel%u", idx);
		  edif_cell_portconfig(cell, wid_r+idx, strdup(cellname),
				       IVL_SIP_INPUT);
	    }

	    for (idx = 0 ;  idx < wid_z ; idx += 1) {
		  unsigned base = wid_r + wid_s + wid_r * idx;
		  unsigned rdx;

		  for (rdx = 0 ;  rdx < wid_r ;  rdx += 1) {
			sprintf(cellname, "Data%ux%u", idx, rdx);
			edif_cell_portconfig(cell, base+rdx, strdup(cellname),
					     IVL_SIP_INPUT);
		  }
	    }

	    edif_cell_pstring(cell,  "LPM_Type",   "LPM_MUX");
	    edif_cell_pinteger(cell, "LPM_Width",  wid_r);
	    edif_cell_pinteger(cell, "LPM_WidthS", wid_s);
	    edif_cell_pinteger(cell, "LPM_Size",   wid_z);
      }


      ref = edif_cellref_create(edf, cell);

	/* Connect the pins of the instance to the nexa. Access the
	   cell pins by name. */
      for (idx = 0 ;  idx < wid_r ;  idx += 1) {
	    unsigned pin;

	    sprintf(cellname, "Result%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, idx));
	    edif_add_to_joint(jnt, ref, pin);
      }

      for (idx = 0 ;  idx < wid_s ;  idx += 1) {
	    unsigned pin;

	    sprintf(cellname, "Sel%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_select(net, idx));
	    edif_add_to_joint(jnt, ref, pin);
      }

      for (idx = 0 ;  idx < wid_z ;  idx += 1) {
	    for (rdx = 0 ;  rdx < wid_r ;  rdx += 1) {
		  unsigned pin;

		  sprintf(cellname, "Data%ux%u", idx, rdx);
		  pin = edif_cell_port_byname(cell, cellname);

		  jnt = edif_joint_of_nexus(edf, ivl_lpm_data2(net, idx, rdx));
		  edif_add_to_joint(jnt, ref, pin);
	    }
      }
}

static void lpm_show_add(ivl_lpm_t net)
{
      unsigned idx;
      unsigned cell_width;
      char cellname[32];
      edif_cell_t cell;
      edif_cellref_t ref;
      edif_joint_t jnt;

      const char*type = "ADD";

      if (ivl_lpm_type(net) == IVL_LPM_SUB)
	    type = "SUB";

	/* Figure out the width of the cell. Normally, it is the LPM
	   width known by IVL. But if the top data input bits are
	   unconnected, then we really have a width one less, and we
	   can use the cout to fill out the output width. */
      cell_width = ivl_lpm_width(net);
      if ( (ivl_lpm_data(net,cell_width-1) == 0)
	   && (ivl_lpm_datab(net,cell_width-1) == 0) )
	    cell_width -= 1;

	/* Find the correct ADD/SUB device in the library, search by
	   name. If the device is not there, then create it and put it
	   in the library. */
      sprintf(cellname, "%s%u", type, cell_width);
      cell = edif_xlibrary_findcell(xlib, cellname);

      if (cell == 0) {
	    unsigned pins = cell_width * 3 + 1;

	    cell = edif_xcell_create(xlib, strdup(cellname), pins);

	    for (idx = 0 ;  idx < cell_width ;  idx += 1) {

		  sprintf(cellname, "Result%u", idx);
		  edif_cell_portconfig(cell, idx*3+0, strdup(cellname),
				       IVL_SIP_OUTPUT);

		  sprintf(cellname, "DataA%u", idx);
		  edif_cell_portconfig(cell, idx*3+1, strdup(cellname),
				       IVL_SIP_INPUT);

		  sprintf(cellname, "DataB%u", idx);
		  edif_cell_portconfig(cell, idx*3+2, strdup(cellname),
				       IVL_SIP_INPUT);
	    }

	    edif_cell_portconfig(cell, pins-1, "Cout", IVL_SIP_OUTPUT);

	    edif_cell_pstring(cell,  "LPM_Type",      "LPM_ADD_SUB");
	    edif_cell_pstring(cell,  "LPM_Direction", type);
	    edif_cell_pinteger(cell, "LPM_Width",     ivl_lpm_width(net));
      }

      ref = edif_cellref_create(edf, cell);

	/* Connect the pins of the instance to the nexa. Access the
	   cell pins by name. */
      for (idx = 0 ;  idx < cell_width ;  idx += 1) {
	    unsigned pin;

	    sprintf(cellname, "Result%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, idx));
	    edif_add_to_joint(jnt, ref, pin);

	    sprintf(cellname, "DataA%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_data(net, idx));
	    edif_add_to_joint(jnt, ref, pin);

	    sprintf(cellname, "DataB%u", idx);
	    pin = edif_cell_port_byname(cell, cellname);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_datab(net, idx));
	    edif_add_to_joint(jnt, ref, pin);
      }

      if (cell_width < ivl_lpm_width(net)) {
	    unsigned pin = edif_cell_port_byname(cell, "Cout");

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, cell_width));
	    edif_add_to_joint(jnt, ref, pin);
      }
}

static void lpm_show_mult(ivl_lpm_t net)
{
      char name[64];
      unsigned idx;

      edif_cell_t cell;
      edif_cellref_t ref;
      edif_joint_t jnt;

      sprintf(name, "mult%u", ivl_lpm_width(net));
      cell = edif_xlibrary_findcell(xlib, name);

      if (cell == 0) {
	    cell = edif_xcell_create(xlib, strdup(name),
				     3 * ivl_lpm_width(net));

	    for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {

		  sprintf(name, "Result%u", idx);
		  edif_cell_portconfig(cell, idx*3+0,
				       strdup(name),
				       IVL_SIP_OUTPUT);

		  sprintf(name, "DataA%u", idx);
		  edif_cell_portconfig(cell, idx*3+1,
				       strdup(name),
				       IVL_SIP_INPUT);

		  sprintf(name, "DataB%u", idx);
		  edif_cell_portconfig(cell, idx*3+2,
				       strdup(name),
				       IVL_SIP_INPUT);
	    }

	    edif_cell_pstring(cell,  "LPM_Type",  "LPM_MULT");
	    edif_cell_pinteger(cell, "LPM_WidthP", ivl_lpm_width(net));
	    edif_cell_pinteger(cell, "LPM_WidthA", ivl_lpm_width(net));
	    edif_cell_pinteger(cell, "LPM_WidthB", ivl_lpm_width(net));
      }

      ref = edif_cellref_create(edf, cell);

      for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	    unsigned pin;
	    ivl_nexus_t nex;

	    sprintf(name, "Result%u", idx);
	    pin = edif_cell_port_byname(cell, name);

	    jnt = edif_joint_of_nexus(edf, ivl_lpm_q(net, idx));
	    edif_add_to_joint(jnt, ref, pin);

	    if ( (nex = ivl_lpm_data(net, idx)) ) {
		  sprintf(name, "DataA%u", idx);
		  pin = edif_cell_port_byname(cell, name);

		  jnt = edif_joint_of_nexus(edf, nex);
		  edif_add_to_joint(jnt, ref, pin);
	    }

	    if ( (nex = ivl_lpm_datab(net, idx)) ) {
		  sprintf(name, "DataB%u", idx);
		  pin = edif_cell_port_byname(cell, name);

		  jnt = edif_joint_of_nexus(edf, nex);
		  edif_add_to_joint(jnt, ref, pin);
	    }
      }

}

static void lpm_show_constant(ivl_net_const_t net)
{
      /* We only need one instance each of constant 0 and 1 bits. If
	 we need either of them, then create an instance reference and
	 save that reference here so that later needs for 0 or 1 can
	 find that the reference already lives and can be added to the
	 joint. */
      static edif_cellref_t cell0_ref = 0;
      static edif_cellref_t cell1_ref = 0;

      static edif_joint_t cell0_jnt = 0;
      static edif_joint_t cell1_jnt = 0;

      edif_cell_t cell0 = edif_xlibrary_findcell(xlib, "cell0");
      edif_cell_t cell1 = edif_xlibrary_findcell(xlib, "cell1");

      const char*bits;
      unsigned idx;

      if (cell0 == 0) {
	    cell0 = edif_xcell_create(xlib, "cell0", 1);
	    edif_cell_portconfig(cell0, 0, "Result0", IVL_SIP_OUTPUT);

	    edif_cell_pstring(cell0,  "LPM_Type",   "LPM_CONSTANT");
	    edif_cell_pinteger(cell0, "LPM_Width",  1);
	    edif_cell_pinteger(cell0, "LPM_CValue", 0);
      }

      if (cell1 == 0) {
	    cell1 = edif_xcell_create(xlib, "cell1", 1);
	    edif_cell_portconfig(cell1, 0, "Result0", IVL_SIP_OUTPUT);

	    edif_cell_pstring(cell1,  "LPM_Type",   "LPM_CONSTANT");
	    edif_cell_pinteger(cell1, "LPM_Width",  1);
	    edif_cell_pinteger(cell1, "LPM_CValue", 1);
      }

      bits = ivl_const_bits(net);
      for (idx = 0 ;  idx < ivl_const_pins(net) ;  idx += 1) {
	    if (bits[idx] == '1') {
		  if (cell1_ref == 0) {
			cell1_ref = edif_cellref_create(edf, cell1);
			cell1_jnt = edif_joint_create(edf);
			edif_add_to_joint(cell1_jnt, cell1_ref, 0);
		  }

	    } else {
		  if (cell0_ref == 0) {
			cell0_ref = edif_cellref_create(edf, cell0);
			cell0_jnt = edif_joint_create(edf);
			edif_add_to_joint(cell0_jnt, cell0_ref, 0);
		  }
	    }
      }

      for (idx = 0 ;  idx < ivl_const_pins(net) ;  idx += 1) {
	    if (bits[idx] == '1')
		  edif_nexus_to_joint(edf, cell1_jnt, ivl_const_pin(net,idx));
	    else
		  edif_nexus_to_joint(edf, cell0_jnt, ivl_const_pin(net,idx));
      }

}


const struct device_s d_lpm_edif = {
      lpm_show_header,
      lpm_show_footer,
      0,
      0, /* show_pad */
      lpm_logic, /* show_logic */
      lpm_show_dff, /* show_dff */
      lpm_show_cmp_eq, /* show_cmp_eq */
      0, /* show_cmp_ne */
      0, /* show_cmp_ge */
      0, /* show_cmp_gt */
      lpm_show_mux, /* show_mux */
      lpm_show_add, /* show_add */
      lpm_show_add, /* show_sub */
      0, /* show_shiftl */
      0, /* show_shiftr */
      lpm_show_mult, /* show_mult */
      lpm_show_constant /* show_constant */
};
