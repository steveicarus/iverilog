#ifndef IVL_priv_H
#define IVL_priv_H
/*
 * Copyright (c) 2000-2014 Stephen Williams (steve@icarus.com)
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

# include  <ivl_target.h>
# include  <ipal.h>


extern pal_t pal;

extern unsigned error_count;

/*
 * A device has an array of pins, that are bound to the netlist either
 * by attribute or by random lookup. The bind_pin table keeps track of
 * pin allocations.
 *
 * Each cell also has attached to it an expression that calculates
 * results from an input. That expression is represented by a sum of
 * product terms. A product term is an array of term_t objects,
 * terminated by a term will a null nex pointers. A sum, then, is an
 * array of pointers to term_t arrays, terminated by a null pointer.
 */

typedef struct term_s {
      int inv;
      ivl_nexus_t nex;
} term_t;

/*
 * This structure describes a target device pin. If the pin is not
 * controlled by the pal (i.e. it is a power pin) then the sop field
 * is null. Otherwise, the sop in the macrocell that controls the pin.
 *
 * If the pin has an enable, then the sop for the enable function is
 * stored here as well.
 *
 * This structure for collecting the PAL design assumes that all the
 * macrocells are associated with pins, or are enables for other
 * pins.
 *
 * The bind_pin array is the complete description of the target as it
 * is accumulated.
 */
struct pal_bind_s {
	/* This is the netlist connection for the pin. */
      ivl_nexus_t nexus;
	/* If the pin is an output, this is is sop that drives it. */
      pal_sop_t sop;

	/* If the output has an enable, this is it, along with the
	   single term that activates it. */
      ivl_net_logic_t enable;
      term_t **enable_ex;

	/* If there is a register here, this is it. */
      ivl_lpm_t reg;
      unsigned reg_q;

	/* The input to the cell is this expression. */
      term_t **sop_ex;
	/* These are the SOP flags that I believe I need. */
      unsigned sop_inv  : 1;
};

extern unsigned pins;
extern struct pal_bind_s* bind_pin;


/*
 * These are various steps in the fitting process.
 */
extern int get_pad_bindings(ivl_scope_t net, void*x);

extern void absorb_pad_enables(void);

extern int fit_registers(ivl_scope_t scope, void*x);

extern int fit_logic(void);

extern int emit_jedec(const char*path);

#endif /* IVL_priv_H */
