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
 */

/*
 * Test the select of a bit from a parameter.
 */
module test;

   reg [4:0] a;
   wire      o;

   RAM dut(o, a[3:0]);
   defparam test.dut.INIT = 16'h55aa;

   initial begin
      for (a = 0 ;  a[4] == 0 ;  a = a + 1) begin
	 #1 $display("dut[%h] = %b", a, o);
      end
   end

endmodule // test

module RAM (O, A);

    parameter INIT = 16'h0000;

    output O;

    input [3:0] A;

    reg  mem [15:0];
    reg  [4:0] count;
    wire [3:0] adr;

    buf (O, mem[A]);

    initial
    begin
	for (count = 0; count < 16; count = count + 1)
	    mem[count] <= INIT[count];
    end

endmodule
