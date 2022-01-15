/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
 * This checks that ! of x works properly.
 */

module main;

   reg x;
   reg [1:0] xx;


   initial begin
      if (1'bx !== 1'bx) begin
	 $display("FAILED -- simple constant x does't compare.");
	 $finish;
      end

      if (1'bx !== !1'bx) begin
	 $display("FAILED -- !1'bx comes out wrong.");
	 $finish;
      end

      x = 1'bx;
      if (x !== 1'bx) begin
	 $display("FAILED -- variable x comes out wrong.");
	 $finish;
      end

      x = !x;
      if (x !== 1'bx) begin
	 $display("FAILED -- ! of variable x comes out wrong.");
	 $finish;
      end

      xx = 2'bx0;
      if (xx !== 2'bx0) begin
	 $display("FAILED -- variable x comes out wrong.");
	 $finish;
      end

      x = !xx;
      if (x !== 1'bx) begin
	 $display("FAILED -- ! of variable xx comes out wrong.");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
