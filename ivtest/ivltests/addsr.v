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
 * This test caught a problem with the addition with different size
 * operands was an operand of the shift.
 */
module main;

   reg [3:0] a;
   reg [4:0] result, b;

   initial begin
      a = 5;
      b = 6;
      result = (a + b) >> 1;
      if (result !== 5) begin
	 $display("FAILED: result === %b", result);
	 $finish;
      end

      $display("PASSED");
   end // initial begin
endmodule // main
