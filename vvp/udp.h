#ifndef IVL_udp_H
#define IVL_udp_H
/*
 * Copyright (c) 2005-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vvp_net.h"
# include  "schedule.h"

struct udp_levels_table;

struct vvp_udp_s {

    public:
      explicit vvp_udp_s(char*label, char*name, unsigned ports,
                         vvp_bit4_t init, bool type);
      virtual ~vvp_udp_s();

	// Return the number of input ports for the defined UDP. This
	// does *not* include the current output value for a
	// sequential UDP.
      unsigned port_count() const;
      bool is_sequential() const { return seq_; };
      char *name() { return name_; }

	// Return the initial output value.
      vvp_bit4_t get_init() const;

      virtual vvp_bit4_t calculate_output(const udp_levels_table&cur,
					  const udp_levels_table&prev,
					  vvp_bit4_t cur_out) =0;

    private:
      char *name_;
      unsigned ports_;
      vvp_bit4_t init_;
      bool seq_;
};

/*
 * The vvp_udp_async_s instance represents a *definition* of a
 * primitive. netlist instances refer to these definitions.
 *
 * The ports argument of the constructor is the number of input ports
 * to the device. The single output port is not counted. The port
 * count must be greater than 0.
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
 * sensitive device is limited to the number of bits in an unsigned
 * long.
 *
 * The array of strings passed to the compile_table method of a
 * combinational UDP are strings of port_count()+1 characters. The
 * expected inputs are in the order of the UDP inputs, and the output
 * is the last character. For example, an AND gate has these strings
 * passed to the compile_table function:
 *
 *    000   (Both inputs are 0)
 *    010   (Second input is a 1)
 *    100
 *    111   (All inputs are 1, so generate a 1 output)
 *
 * The characters allowed in the input positions are:
 *
 *   0  -- Expect a 0
 *   1  -- Expect a 1
 *   x  -- Expect an x or z
 *   b  -- 0 or 1
 *   l  -- 0 or x
 *   h  -- x or 1
 *   ?  -- 0, x or 1
 *
 * Only 0, 1 and x characters are allowed in the output position.
 */

struct udp_levels_table {
      unsigned long mask0;
      unsigned long mask1;
      unsigned long maskx;
};
extern std::ostream& operator<< (std::ostream&o, const struct udp_levels_table&t);

class vvp_udp_comb_s : public vvp_udp_s {

    public:
      vvp_udp_comb_s(char*label, char*name__, unsigned ports);
      ~vvp_udp_comb_s();
      void compile_table(char**tab);

	// Test the cur table with the compiled rows, and return the
	// bit value that matches.
      vvp_bit4_t test_levels(const udp_levels_table&cur);

      vvp_bit4_t calculate_output(const udp_levels_table&cur,
				  const udp_levels_table&prev,
				  vvp_bit4_t cur_out);

    private:
	// Level sensitive rows of the device.
      struct udp_levels_table*levels0_;
      struct udp_levels_table*levels1_;
      unsigned nlevels0_, nlevels1_;
};

/*
 * udp sequential devices are a little more complex in that they have
 * an additional output type: no-change. They also have, in addition
 * to a table of levels, a table of edges that are similar to levels
 * but one of the positions has an edge value.
 *
 * The port_count() for the device is the number of inputs. Sequential
 * devices have an additional phantom port that is the current output
 * value. This implies that the maximum port count for sequential
 * devices is 1 less than for combinational.
 *
 * The input table that is passed to the compile_table method is very
 * similar to that for the combinational UDP. The differences are that
 * there is one extra entry in the beginning of each input row that is
 * the current output, and there are more characters accepted at each
 * bit position.
 *
 * The current output bit may contain the same characters as a
 * combinational UDP input position. No edges.
 *
 * The next output position takes the possible values 0, x, 1 and -.
 *
 * The input bit positions take the combinational input characters,
 * plus edge specification characters. Only one input of a row may
 * have an edge input.
 *
 * The edges_table is similar to the levels, table, with the mask?
 * bits having the same meaning as with the levels tables. The edge_*
 * members specify the edge for a single bit position. In this case,
 * the edge_mask* members give the mask of source bits for that
 * position, and the edge_position the bit that has shifted. In the
 * edge case, the mask* members give the final position and the
 * edge_mask* bits the initial position of the bit.
 */
struct udp_edges_table {
      unsigned long edge_position : 8;
      unsigned long edge_mask0 : 1;
      unsigned long edge_mask1 : 1;
      unsigned long edge_maskx : 1;
      unsigned long mask0;
      unsigned long mask1;
      unsigned long maskx;
};

class vvp_udp_seq_s : public vvp_udp_s {

    public:
      vvp_udp_seq_s(char*label, char*name__, unsigned ports, vvp_bit4_t init);
      ~vvp_udp_seq_s();

      void compile_table(char**tab);

      vvp_bit4_t calculate_output(const udp_levels_table&cur,
				  const udp_levels_table&prev,
				  vvp_bit4_t cur_out);

    private:
      vvp_bit4_t test_levels_(const udp_levels_table&cur);

	// Level sensitive rows of the device.
      struct udp_levels_table*levels0_;
      struct udp_levels_table*levels1_;
      struct udp_levels_table*levelsx_;
      struct udp_levels_table*levelsL_;
      unsigned nlevels0_, nlevels1_, nlevelsx_, nlevelsL_;

      vvp_bit4_t test_edges_(const udp_levels_table&cur,
			     const udp_levels_table&prev);

	// Edge sensitive rows of the device
      struct udp_edges_table*edges0_;
      struct udp_edges_table*edges1_;
      struct udp_edges_table*edgesL_;
      unsigned nedges0_, nedges1_, nedgesL_;

};

/*
 * This looks up a UDP definition from its LABEL.
 */
struct vvp_udp_s *udp_find(const char *label);

/*
 * The udp_fun_core is the core of the udp instance in the
 * netlist. This core is a vvp_wide_fun_core that takes care of
 * dispatching the output. This class also receives the inputs from
 * the vvp_wide_fun_t objects and processes them to generate the
 * output to be sent.
 */
class vvp_udp_fun_core  : public vvp_wide_fun_core, private vvp_gen_event_s {

    public:
      vvp_udp_fun_core(vvp_net_t*net, vvp_udp_s*def);
      ~vvp_udp_fun_core();

      void recv_vec4_from_inputs(unsigned);

    private:
      void run_run();

      vvp_udp_s*def_;
      vvp_bit4_t cur_out_;
      udp_levels_table current_;
};

#endif /* IVL_udp_H */
