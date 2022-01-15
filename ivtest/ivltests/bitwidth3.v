/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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


module main();

   reg [4:0] sum;

   initial begin

      // Self-determined expressions take their size from the
      // operands. The compiler should thus make the constant
      // expression below exactly 4 bits wide.

      $display("Should be 0001: %b", 4'd7 + 4'd10);

      if ($bits(4'd7 + 4'd10) != 4) begin
	 $display("FAILED -- bit width should be 4: %d",
		  $bits(4'd7 + 4'd10));
	 $finish;
      end

      // When assigning to an l-value, the expression, and
      // by extension the operands, take on the width of the
      // left side. This expansion should be passed all the
      // way down the expression.

      sum = 4'd7 + 4'd10;
      $display("Should be 10001: %b", sum);

      if (sum !== 5'b1_0001) begin
	 $display("FAILED -- expression truncated?");
	 $finish;
      end

      $display("PASSED");
   end


endmodule
