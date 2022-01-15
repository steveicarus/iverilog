/*
 * Copyright (c) 2000 Stephan I. Boettcher <stephan@nevis.columbia.edu>
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

// compile time test for nested task scope elaboration

`define TEST

module nest(r);

   output [7:0] r;
   reg [7:0] r;

   task incr;
      input [7:0] a;
      begin
	 r <= r+a;
	 #1 $display("R=%b",r);
      end
   endtask

endmodule

module test;

   wire [7:0] acc;

   nest n(acc);

   initial n.r <= 0;

`ifdef TEST
   task increment;
      begin
	 n.incr(1);
      end
   endtask
`endif

   initial
     begin
`ifdef TEST
	#10 increment;
	#10 increment;
	#10 increment;
`else
	#10 n.incr(3);
`endif
	#10;
	if (acc==3)
	  $display("PASSED");
	else
	  $display("FAILED");
     end

endmodule
