/*
 * Copyright (c) 2000 Steve Wilson (stevew@home.com)
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
 * This checks bit select from/to vectors with odd bit arrangements.
 */
module test;

   reg [4:1] a;
   reg [1:4] b;
   integer   i;

   initial begin
      a = 4'b1100;
      for (i = 1 ;  i <= 4 ;  i = i + 1)
	b[i] = a[i];

      $display("a=%b, b=%b", a, b);
      if (b !== 4'b0011) begin
	 $display("FAILED -- b == %b", b);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
