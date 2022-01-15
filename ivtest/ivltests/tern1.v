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
 * This test catches the case where the operands of the ?: operator
 * have different sizes.
 */
module main;

   reg [3:0] r;
   reg [3:0] a;
   reg [4:0] b;
   reg	     f;


   initial begin
      a =  4'b1010;
      b = 5'b10101;

      f = 1;
      r = f? a : b;
      if (r !== 4'b1010) begin
	 $display("FAILED: r === %b", r);
	 $finish;
      end

      f = 0;
      r = f? a : b;
      if (r !== 4'b0101) begin
	 $display("FAILED: r === %b", r);
	 $finish;
      end

      $display("PASSED");

   end // initial begin
endmodule // main
