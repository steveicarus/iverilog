/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
 * This tests the primitive synthesis of a simple left shift.
 */

module test (clk,c,a,b);
input clk;
input a, b;
output [1:0] c;
reg [1:0] c;

(* ivl_synthesis_on *)
always @(posedge clk)
    c <= (a << 1) | b;

endmodule

module main;

   reg a, b, clk;
   wire [1:0] c;
   test dut (.clk(clk), .c(c), .a(a), .b(b));

   integer    x;
   (* ivl_synthesis_off *)
   initial begin
      clk = 0;
      for (x = 0 ;  x < 4 ;  x = x+1) begin
	 a = x[1];
	 b = x[0];
	 #1 clk = 1;
	 #1 clk = 0;
	 if (c !== x[1:0]) begin
	    $display("FAILED == x=%0d (ab=%b%b), c=%b", x, a, b, c);
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
