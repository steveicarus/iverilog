#ifndef __priv_H
#define __priv_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: priv.h,v 1.2 2000/12/09 03:42:52 steve Exp $"
#endif

# include  <ivl_target.h>
# include  <ipal.h>

extern pal_t pal;

extern unsigned error_count;

/*
 * A device has an array of pins, that are bound to the netlist either
 * by attribute or by random lookup. The bind_pin table keeps track of
 * pin allocations.
 */
struct pal_bind_s {
	/* This is the netlist connection for the pin. */
      ivl_nexus_t nexus;
	/* If the pin is an output, this is is sop that drives it. */
      pal_sop_t sop;
	/* If the output has an enable, this is it. */
      ivl_net_logic_t enable;
	/* If there is a register here, this is it. */
      ivl_lpm_ff_t reg;
      unsigned reg_q;
};

extern unsigned pins;
extern struct pal_bind_s* bind_pin;

extern int get_pad_bindings(ivl_scope_t net);

extern void absorb_pad_enables(void);

extern int fit_registers(ivl_scope_t scope);

/*
 * $Log: priv.h,v $
 * Revision 1.2  2000/12/09 03:42:52  steve
 *  Stuff registers into macrocells.
 *
 * Revision 1.1  2000/12/09 01:17:38  steve
 *  Add the pal loadable target.
 *
 */
#endif
