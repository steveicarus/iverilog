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

module main;

   wire [31:0] A;
   wire [24:0] B;
   reg [15:0]  C;

   assign      A = B;
   assign      B = C;

   initial begin
      C = 0;
      #1 if (A !== 32'h0) begin
	 $display("FAILED -- A === %h", A);
	 $finish;
      end

      C = -1;
      #1 if (A !== 32'h00_00_ff_ff) begin
	 $display("FAILED -- A == %h instead of 0000ffff", A);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
