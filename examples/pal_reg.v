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

/*
 * This example shows how to use Icarus Verilog to generate PLD output.
 * The design is intended to fit into a 22v10 in a PLCC package, with
 * pin assignments locked down by design.
 */

/*
 * The register module is an 8 bit register that copies the input to
 * the output registers on the rising edge of the clk input. The output
 * drivers are controled by a single active low output enable.
 *
 * This module contains all the logic of the device, but includes nothing
 * that has anything to do with the real hardware.
 */
module register (out, val, clk, oe);

   output [7:0] out;
   input [7:0] 	val;
   input 	clk, oe;

   reg [7:0] 	Q;

   wire [7:0] 	out;

   bufif0 drv[7:0](out, Q, oe);

   always @(posedge clk) Q = val;

endmodule


/*
 * The module pal is used to attach pin information to all the pins of
 * the device. We use this to lock down the pin assignments of the
 * synthesized result. The pin number assignments are for a 22v10 in
 * a PLCC package.
 */
module pal;

   wire out7, out6, out5, out4, out3, out2, out1, out0;
   wire inp7, inp6, inp5, inp4, inp3, inp2, inp1, inp0;
   wire clk, oe;

   // The PAD attributes attach the wires to pins of the
   // device. Output pins are prefixed by a 'o', and input pins by an
   // 'i'. If not all the available output pins are used, then the
   // remaining are available for the synthesizer to drop internal
   // registers or extra logic layers.
   $attribute(out7, "PAD", "o27");
   $attribute(out6, "PAD", "o26");
   $attribute(out5, "PAD", "o25");
   $attribute(out4, "PAD", "o24");
   $attribute(out3, "PAD", "o23");
   $attribute(out2, "PAD", "o21");
   $attribute(out1, "PAD", "o20");
   $attribute(out0, "PAD", "o19");

   $attribute(inp7, "PAD", "i10");
   $attribute(inp6, "PAD", "i9");
   $attribute(inp5, "PAD", "i7");
   $attribute(inp4, "PAD", "i6");
   $attribute(inp3, "PAD", "i5");
   $attribute(inp2, "PAD", "i4");
   $attribute(inp1, "PAD", "i3");
   $attribute(inp0, "PAD", "i2");

   //$attribute(clk, "PAD", "CLK");
   $attribute(oe,  "PAD", "i13");

   register dev({out7, out6, out5, out4, out3, out2, out1, out0},
		{inp7, inp6, inp5, inp4, inp3, inp2, inp1, inp0},
		clk, oe);

endmodule // pal
