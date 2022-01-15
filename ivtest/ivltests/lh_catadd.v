/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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

module test;

   wire [3:0] a = 7, b = 13 ;
   wire [3:0] sum ;
   wire carry ;
   assign {carry,sum} = a + b ;

   initial begin
      #1
      if (carry !== 1'b1) begin
	 $display("FAILED: carry === %b", carry);
	 $finish;
      end

      if (sum !== 4'b0100) begin
	 $display("FAILED: sum === %b", sum);
	 $finish;
      end

      $display("Correct results {carry,sum} === %b,%b", carry, sum);
      $display("PASSED");
   end
endmodule /* test */
