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
 * This program tests the behavioral modulus operator, to make sure it
 * works at least minimally.
 */

module main;

   reg [15:0] a, b, c;

   initial begin
      a = 1;
      b = 1;
      c = a % b;

      if (c !== 16'd0) begin
	 $display("FAILED -- 1 %% 1 == 'b%b", c);
	 $finish;
      end

      a = 9;
      b = 8;
      c = a % b;

      if (c !== 16'd1) begin
	 $display("FAILED -- 9 %% 8 == 'b%b", c);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
