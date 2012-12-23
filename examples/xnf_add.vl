/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
 * This example demonstrates Icarus Verilog's ability to synthesize
 * efficient adders for Xilinx 4000 series FPGAs. The synthesis and
 * code generation makes an adder and translates that into XOR gates
 * and fast carry chain hardware.
 *
 * To compile this for XNF, try a command like this:
 *
 *    iverilog -txnf -ppart=XC4010XLPQ160 -pncf=xnf_add.ncf -oxnf_add.xnf xnf_add.v
 *
 * That command causes an xnf_add.xnf and xnf_add.ncf file to be created.
 * Next, Use Xilinx Alliance or Foundation tools to make the xnf_add.ngd
 * file with the command:
 *
 *    xnf2ngd -l xilinxun -u xnf_add.xnf xnf_add.ngo
 *    ngdbuild xnf_add.ngo xnf_add.ngd
 *
 * Finally, map the file to fully render it in the target part. The
 * par command is the step that actually optimizes the design and tries
 * to meet timing constraints.
 *
 *    map -o map.ncd xnf_add.ngd
 *    par -w map.ncd xnf_add.ncd
 *
 * At this point, you can use the Xilinx FPGA Editor to edit the xnf_add.ncd
 * file and see the carry chains made up to support the adder.
 */

module main;

   wire [3:0] a, b;
   wire [3:0] out;
   wire carry;

   wire a0, a1, a2, a3, b0, b1, b2, b3;
   wire out0, out1, out2, out3;

   // This creates the actual adder. Note that we also create a link
   // to the carry output. The principle adder is 4 bits wide, so two
   // IOBs are used to to the actual addition. PAR will place them in
   // order along carry lines. An extra carry cell from below a[0] is
   // used in FORCE-0 mode to load the bottom carry node.
   //
   // The carry signal from the top CY device is used to drive the
   // main.carry wire shown here. It is managed in this case by using
   // an ADD-FG-CI CY device for the top pair and using the CLB above
   // the carry chain, with its CY in EXAMINE-CI mode, to put the carry
   // out through its G function unit.

   assign {carry, out} = a + b;


   // These attribute commands assign pins to the listed wires.
   // This can be done to wires and registers, as internally both
   // are treated as named signals. It doesn't work (yet) on vectors,
   // though, so break out the vectors with scalar assignments.

   assign a[0] = a0;
   assign a[1] = a1;
   assign a[2] = a2;
   assign a[3] = a3;
   $attribute(a0, "PAD", "i150");
   $attribute(a1, "PAD", "i152");
   $attribute(a2, "PAD", "i153");
   $attribute(a3, "PAD", "i154");
   assign b[0] = b0;
   assign b[1] = b1;
   assign b[2] = b2;
   assign b[3] = b3;
   $attribute(b0, "PAD", "i155");
   $attribute(b1, "PAD", "i156");
   $attribute(b2, "PAD", "i157");
   $attribute(b3, "PAD", "i158");
   assign out0 = out[0];
   assign out1 = out[1];
   assign out2 = out[2];
   assign out3 = out[3];
   $attribute(out0,  "PAD", "o71");
   $attribute(out1,  "PAD", "o72");
   $attribute(out2,  "PAD", "o73");
   $attribute(out3,  "PAD", "o74");
   $attribute(carry, "PAD", "o75");

endmodule /* main */
