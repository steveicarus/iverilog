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
 * This test is intended to be run with ``iverilog -S foo.v'',
 * and tests the situation addressed by pr#519.
 */
module main;

  reg [3:0] a, b, c, t;

  (* ivl_combinational *)
  always @(a, b) begin
     t = a + b;
     c = 4'd1 + ~t;
  end

  (* ivl_synthesis_off *)
  initial begin
     a = 1;
     for (b = 0 ;  b < 4'hf ;  b = b + 1) begin
	#1 if (c !== -(a + b)) begin
	   $display("FAILED -- a=%b, b=%b, t=%b, c=%b", a, b, t, c);
	   $finish;
	end
     end

     $display("PASSED");
  end // initial begin

endmodule
