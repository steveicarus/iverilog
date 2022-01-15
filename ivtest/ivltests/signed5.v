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
 * This program tests the magic $signed system function.
 */

module main;

   reg [3:0] a;

   initial begin
      a = 4'd12;

      // The expression should not change the bit pattern in any way
      if ($signed(a) !== 4'b1100) begin
	 $display("FAILED -- $signed(%b) === %b", a, $signed(a));
	 $finish;
      end

      if ($signed(a) == 4) begin
	 $display("FAILED -- $signed(%b) == 4", a);
	 $finish;
      end

      // The >= should do a signed comparison here.
      if ($signed(a) >= 0) begin
	 $display("FAILED -- $signed(%b) > 0", a);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
