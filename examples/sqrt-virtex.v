/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
 *
 *   $Id: sqrt-virtex.v,v 1.4.2.2 2005/02/23 18:37:52 steve Exp $"
 */

/*
 * This module is a synthesizeable square-root function. It is also a
 * detailed example of how to target Xilinx Virtex parts using
 * Icarus Verilog. In fact, for no particular reason other than to
 * be excessively specific, I will step through the process of
 * generating a design for a Spartan-II XC2S15-VQ100, and also how to
 * generate a generic library part for larger Virtex designs.
 *
 * In addition to Icarus Verilog, you will need implementation
 * software from Xilinx. As of this writing, this example was tested
 * with Foundation 4.2i, but it should work the same with ISE and
 * Webpack software.
 *
 * This example source contains all the Verilog needed to do
 * everything described below. We use conditional compilation to
 * select the bits of Verilog that are needed to perform each specific
 * task.
 *
 * SIMULATE THE DESIGN
 *
 * This source file includes a simulation test bench. To compile the
 * program to include this test bench, use the command line:
 *
 *    iverilog -DSIMULATE=1 -oa.out sqrt-virtex.v
 *
 * This generates the file "a.out" that can then be executed with the
 * command:
 *
 *    vvp a.out
 *
 * This causes the simulation to run a long set of example sqrt
 * calculations. Each result is checked by the test bench to assure
 * that the result is valid. When it is done, the program prints
 * "PASSED" and finishes the simulation.
 *
 * When you take a close look at the "main" module below, you will see
 * that it uses Verilog constructs that are not synthesizeable. This
 * is fine, as we will never try to synthesize it.
 *
 * LIBRARY PARTS
 *
 * One can use the sqrt32 module to generate an EDIF file suitable for
 * use as a library part. This part can be imported to the Xilinx
 * schematic editor, then placed like any other pre-existing
 * macro. One can also pass the generated EDIF as a precompiled macro
 * that other designers may use as they see fit.
 *
 * To make an EDIF file from the sqrt32 module, execute the command:
 *
 *    iverilog -osqrt32.edf -tfpga -parch=virtex sqrt-virtex.v
 *
 * The -parch=virtex tells the code generator to generate code for the
 * virtex architecture family (we don't yet care what specific part)
 * and the -osqrt32.edf places the output into the file
 * sqrt32.edf.
 *
 * Without any preprocessor directives, the only module is the sqrt32
 * module, so sqrt32 is compiled as the root. The ports of the module
 * are automatically made into ports of the sqrt32.edf netlist, and
 * the contents of the sqrt32 module are connected approprately.
 *
 * COMPLETE CHIP DESIGNS
 *
 * To make a complete chip design, there are other bits that need to
 * be accounted for. Signals must be assigned to pins, and some
 * special devices may need to be created. We also want to write into
 * the EDIF file complete part information so that the implementation
 * tools know how to route the complete design. The command to compile
 * for our target part is:
 *
 *    iverilog -ochip.edf -tfpga \
 *           -parch=virtex -ppart=XC2S15-VQ100 \
 *           -DMAKE_CHIP=1 sqrt-virtex.v
 *
 * This command uses the "chip" module as the root. This module in
 * turn has ports that are destined to be the pins of the completed
 * part. The -ppart= option gives complete part information, that is
 * in turn written into the EDIF file. This saves us the drudgery of
 * repeating that part number for later commands.
 *
 * The next steps involve Xilinx software, and to talk to Xilinx
 * software, the netlist must be in the form of an "ngd" file, a
 * binary netlist format. The command:
 *
 *    ngdbuild chip.edf chip.ngd
 *
 * does the trick. The input to ngdbuild is the chip.edf file created
 * by Icarus Verilog, and the output is the chip.ngd file that the
 * implementation tools may read. From this point, it is best to refer
 * to Xilinx documentation for the software you are using, but the
 * quick summary is:
 *
 *    map -o map.ncd chip.ngd
 *    par -w map.ncd chip.ncd
 *
 * The result of this sequence of commands is the chip.ncd file that
 * is ready to be viewed by FPGA Edit, or converted to a bit stream,
 * or whatever.
 *
 * POST MAP SIMULATION
 *
 * Warm fuzzies are good, and retesting your design after the part
 * is mapped by the Xilinx backend tools is a cheap source of fuzzies.
 * The command to make a Verilog file out of the mapped design is:
 *
 *    ngd2ver chip.ngd chip_root.v
 *
 * This command creates from the chip.ngd the file "chip_root.v" that
 * contains Verilog code that simulates the mapped design. This output
 * Verilog has the single root module "chip_root", which came from the
 * name of the root module when we were making the EDIF file in the
 * first place. The module has ports named just like the ports of the
 * chip_root module below.
 *
 * The generated Verilog uses the library in the directory
 * $(XILINX)/verilog/src/simprims. This directory comes with the ISE
 * WebPACK installation that you are using. Icarus Verilog is able to
 * simulate using that library.
 *
 * To compile a post-map simulation of the chip_root.v, use the
 * command:
 *
 *    iverilog -DSIMULATE -DPOST_MAP -ob.out \
 *             -y $(XILINX)/verilog/src/simprims \
 *             sqrt-virtex.v chip_root.v \
 *             $(XILINX)/verilog/src/glbl.v
 *
 * This command line generates b.out from the source files
 * sqrt-virtex.v and chip_root.v (the latter from ngd2ver)
 * and the "-y <path>" flag specifies the library directory that will
 * be needed. The glbl.v source file is also included to provide the
 * GSR and related signals.
 *
 * The POST_MAP compiler directive causes the GSR manipulations
 * included in the test bench to be compiled in, to simulate the chip
 * startup. Other then that, the test bench runs the post-map design
 * the same way the pre-synthesis design works.
 *
 * Run this design with the command:
 *
 *    vvp b.out
 *
 * And there you go.
 */

`ifndef POST_MAP
 /*
  * This module approximates the square root of an unsigned 32bit
  * number. The algorithm works by doing a bit-wise binary search.
  * Starting from the most significant bit, the accumulated value
  * tries to put a 1 in the bit position. If that makes the square
  * too big for the input, the bit is left zero, otherwise it is set
  * in the result. This continues for each bit, decreasing in
  * significance, until all the bits are calculated or all the
  * remaining bits are zero.
  *
  * Since the result is an integer, this function really calculates
  * value of the expression:
  *
  *      x = floor(sqrt(y))
  *
  * where sqrt(y) is the exact square root of y and floor(N) is the
  * largest integer <= N.
  *
  * For 32bit numbers, this will never run more then 16 iterations,
  * which amounts to 16 clocks.
  */

module sqrt32(clk, rdy, reset, x, .y(acc));
   input  clk;
   output rdy;
   input  reset;

   input [31:0] x;
   output [15:0] acc;


   // acc holds the accumulated result, and acc2 is the accumulated
   // square of the accumulated result.
   reg [15:0] acc;
   reg [31:0] acc2;

   // Keep track of which bit I'm working on.
   reg [4:0]  bitl;
   wire [15:0] bit = 1 << bitl;
   wire [31:0] bit2 = 1 << (bitl << 1);

   // The output is ready when the bitl counter underflows.
   wire rdy = bitl[4];

   // guess holds the potential next values for acc, and guess2 holds
   // the square of that guess. The guess2 calculation is a little bit
   // subtle. The idea is that:
   //
   //      guess2 = (acc + bit) * (acc + bit)
   //             = (acc * acc) + 2*acc*bit + bit*bit
   //             = acc2 + 2*acc*bit + bit2
   //             = acc2 + 2 * (acc<<bitl) + bit
   //
   // This works out using shifts because bit and bit2 are known to
   // have only a single bit in them.
   wire [15:0] guess  = acc | bit;
   wire [31:0] guess2 = acc2 + bit2 + ((acc << bitl) << 1);

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge reset)
      if (reset) begin
	 acc = 0;
	 acc2 = 0;
	 bitl = 15;
      end else begin
	 if (guess2 <= x) begin
	    acc  <= guess;
	    acc2 <= guess2;
	 end
	 bitl <= bitl - 5'd1;
      end

endmodule // sqrt32

`endif //  `ifndef POST_MAP

`ifdef SIMULATE
 /*
  * This module is a test bench for the sqrt32 module. It runs some
  * test input values through the sqrt32 module, and checks that the
  * output is valid. If an invalid output is generated, print and
  * error message and stop immediately. If all the tested values pass,
  * then print PASSED after the test is complete.
  */
module main;

   reg [31:0] x;
   reg 	      clk, reset;

   wire [15:0] y;
   wire        rdy;

`ifdef POST_MAP
   chip_root dut(.clk(clk), .reset(reset), .rdy(rdy), .x(x), .y(y));
`else
   sqrt32 dut(.clk(clk), .reset(reset), .rdy(rdy), .x(x), .y(y));
`endif

   (* ivl_synthesis_off *)
   always #5 clk = !clk;

   task reset_dut;
      begin
	 reset = 1;
	 @(posedge clk) ;
	 #1 reset = 0;
	 @(negedge clk) ;
      end
   endtask // reset_dut

   task crank_dut;
      begin
	 while (rdy == 0) begin
	    @(posedge clk) /* wait */;
	 end
      end
   endtask // crank_dut

`ifdef POST_MAP
   reg        GSR;
   assign      glbl.GSR = GSR;
`endif

   integer idx;

   (* ivl_synthesis_off *)
   initial begin
      reset = 0;
      clk = 0;

      /* If doing a post-map simulation, when we need to wiggle
         The GSR bit to simulate chip power-up. */
`ifdef POST_MAP
      GSR = 1;
      #100 GSR = 0;
`endif
      #100 x = 1;
      reset_dut;
      crank_dut;
      $display("x=%d, y=%d", x, y);

      x = 3;
      reset_dut;
      crank_dut;
      $display("x=%d, y=%d", x, y);

      x = 4;
      reset_dut;
      crank_dut;
      $display("x=%d, y=%d", x, y);

      for (idx = 0 ;  idx < 200 ;  idx = idx + 1) begin
	 x = $random;
	 reset_dut;
	 crank_dut;
	 $display("x=%d, y=%d", x, y);

	 if (x < (y * y)) begin
	    $display("ERROR: y is too big");
	    $finish;
	 end

	 if (x > ((y + 1)*(y + 1))) begin
	    $display("ERROR: y is too small");
	    $finish;
	 end
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
`endif

`ifdef MAKE_CHIP
 /*
  * This module represents the chip packaging that we intend to
  * generate. We bind pins here, and route the clock to the global
  * clock buffer.
  */
module chip_root(clk, rdy, reset, x, y);
   input  clk;
   output rdy;
   input  reset;

   input [31:0] x;
   output [15:0] y;

   wire 	 clk_int;

   (* cellref="BUFG:O,I" *)
   buf gbuf (clk_int, clk);

   sqrt32 dut(.clk(clk_int), .reset(reset), .rdy(rdy), .x(x), .y(y));

   /* Assign the clk to GCLK0, which is on pin P39. */
   $attribute(clk, "PAD", "P39");

   // We don't care where the remaining pins go, so set the pin number
   // to 0. This tells the implementation tools that we want a PAD,
   // but we don't care which. Also note the use of a comma (,)
   // separated list to assign pins to the bits of a vector.
   $attribute(rdy, "PAD", "0");
   $attribute(reset, "PAD", "0");
   $attribute(x, "PAD", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
   $attribute(y, "PAD", "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");

endmodule // chip_root

`endif
