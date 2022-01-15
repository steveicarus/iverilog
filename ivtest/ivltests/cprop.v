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
 * This test triggers constant propagation through AND gates.
 */
module main;

   wire a = 1'b0;
   wire b = 1'b1;
   wire c = 1'b1;
   wire d = 1'bx;

   wire out0, out1, out2, out3;

   and (out0, a, b); // Should be 0
   and (out1, b, c); // Should be 1
   and (out2, a, d); // Should be 0 because of a
   and (out3, b, d); // Should be x

   initial begin
      #0 if (out0 !== 1'b0) begin
	 $display("FAILED -- out0 = %b", out0);
	 $finish;
      end

      if (out1 !== 1'b1) begin
	 $display("FAILED -- out1 = %b", out1);
	 $finish;
      end

      if (out2 !== 1'b0) begin
	 $display("FAILED -- out2 = %b", out2);
	 $finish;
      end

      if (out3 !== 1'bx) begin
	 $display("FAILED -- outx = %b", out3);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
