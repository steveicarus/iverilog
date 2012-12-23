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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * This source file demonstrates how to synthesize CLB flip-flops from
 * Icarus Verilog, including giving the device an initial value.
 *
 * To compile this for XNF, try a command like this:
 *
 *    iverilog -txnf -ppart=XC4010XLPQ160 -pncf=clbff.ncf -oclbff.xnf clbff.v
 *
 * That command causes an clbff.xnf and clbff.ncf file to be created.
 * Next, make the clbff.ngd file with the command:
 *
 *    xnf2ngd -l xilinxun -u clbff.xnf clbff.ngo
 *    ngdbuild clbff.ngo clbff.ngd
 *
 * Finally, map the file to fully render it in the target part. The
 * par command is the step that actually optimizes the design and tries
 * to meet timing constraints.
 *
 *    map -o map.ncd clbff.ngd
 *    par -w map.ncd clbff.ncd
 *
 * At this point, you can use the FPGA Editor to edit the clbff.ncd
 * file. Notice that the design uses two CLB flip-flops (possibly in
 * the same CLB) with their outputs ANDed together. If you go into the
 * block editor, you will see that the FF connected to main/Q<0> is
 * configured so start up reset, and the FF connected to main/Q<1> is
 * configured to start up set.
 */

module main;

   wire clk, iclk;
   wire i0, i1;
   wire out;

   wire [1:0] D = {i1, i0};

      // This statement declares Q to be a 2 bit reg vector. The
      // initial assignment will cause the synthesized device to take
      // on an initial value specified here. Without the assignment,
      // the initial value is unspecified. (Verilog simulates it as 2'bx.)
   reg  [1:0] Q = 2'b10;

      // This simple logic gate get turned into a function unit.
      // The par program will map this into a CLB F or G unit.
   and (out, Q[0], Q[1]);

     // This creates a global clock buffer. Notice how I attach an
     // attribute to the named gate to force it to be mapped to the
     // desired XNF device. This device will not be pulled into the
     // IOB associated with iclk because of the attribute.
   buf gbuf(clk, iclk);
   $attribute(gbuf, "XNF-LCA", "GCLK:O,I");

      // This is mapped to a DFF. Since Q and D are two bits wide, the
      // code generator actually makes two DFF devices that share a
      // clock input.
   always @(posedge clk) Q <= D;

      // These attribute commands assign pins to the listed wires.
      // This can be done to wires and registers, as internally both
      // are treated as named signals.
   $attribute(out, "PAD", "o150");
   $attribute(i0,  "PAD", "i152");
   $attribute(i1,  "PAD", "i153");
   $attribute(iclk,"PAD", "i154");

endmodule /* main */
