#ifndef __udp_H
#define __udp_H
/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
 *
 * (This is a rewrite of code that was ...
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>)
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
#ident "$Id: udp.h,v 1.15 2005/04/01 06:02:45 steve Exp $"
#endif

# include  <vvp_net.h>

/*
 * The vvp_udp_s instance represents a *definition* of a
 * primitive. netlist instances refer to these definitions.
 *
 * The ports argument of the constructor is the number of input ports
 * to the device. The single output port is not counted. The port
 * count must be greater then 0.
 *
 * A level sensitive UDP has a table that includes all the rows that
 * generate a 0 output and another table that includes all the rows
 * that generate a 1 output. A set of inputs is tested against the
 * entries in both sets, and if there are no matches, the output is
 * set to x.
 *
 * The levels0 and levels1 tables are each an array of levels
 * tables. Each levels table is a mask of positions that are supposed
 * to be 0, 1 and x. The LSB of each mask represents first port, and
 * so on. If the bit is set in mask0, a bit4_0 is expected at that
 * position, and similarly for mask1 and maskx. Only exactly 1 bit
 * will be set in the three masks for each bit position.
 *
 * This table structure implies that the number of inputs to the level
 * sensitive device is limited to the number of bits in an unsigned long.
 */

struct udp_levels_table {
      unsigned long mask0;
      unsigned long mask1;
      unsigned long maskx;
};

class vvp_udp_s {

    public:
      vvp_udp_s(char*label, char*name, unsigned ports, bool sequ);
      ~vvp_udp_s();
      void compile_table(char**tab);

	// Return the number of input ports for the defined UDP.
      unsigned port_count() const;

	// Test the cur table with the compiled rows, and return the
	// bit value that matches.
      vvp_bit4_t test_levels(const udp_levels_table&cur);

    private:
      char*name_;
      unsigned ports_;

	// Level sensitive rows of the device.
      struct udp_levels_table*levels0_;
      struct udp_levels_table*levels1_;
      unsigned nlevels0_, nlevels1_;
};

/*
 * Ths looks up a UDP definition from its LABEL.
 */
struct vvp_udp_s *udp_find(const char *label);

/*
 * The udp_fun_core is the core of the udp instance in the
 * netlist. This core is a vvp_wide_fun_core that takes care of
 * dispatching the output. This class also receives the inputs from
 * the vvp_wide_fun_t objects and processes them to generate the
 * output to be sent.
 */
class vvp_udp_fun_core  : public vvp_wide_fun_core {

    public:
      vvp_udp_fun_core(vvp_net_t*net, vvp_udp_s*def);
      ~vvp_udp_fun_core();

      void recv_vec4_from_inputs(unsigned);

    private:
      vvp_udp_s*def_;
      udp_levels_table current_;
};



/*
 * $Log: udp.h,v $
 * Revision 1.15  2005/04/01 06:02:45  steve
 *  Reimplement combinational UDPs.
 *
 */
#endif
