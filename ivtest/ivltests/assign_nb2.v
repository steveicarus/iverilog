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
 * Catch problems with non-zero lsb values in l-value expressions.
 */
module main;

   reg [7:1] a  = 6'b111111;
   reg [7:1] b = 6'b000010;

   integer q;
   reg [7:1] x;
   reg	     PCLK = 1;
   always @(posedge PCLK)
     for (q=1; q<=7; q=q+1)
       x[q] <= #1 a[q] & b[q];

   always #5 PCLK = !PCLK;

   initial begin
//      $dumpfile("dump.vcd");
//      $dumpvars(0, main);
      #50 $display("done: x=%b", x);

      if (x !== 6'b000010)
	$display("FAILED -- x = %b", x);
      else
	$display("PASSED");

      $finish;
   end


endmodule // main
