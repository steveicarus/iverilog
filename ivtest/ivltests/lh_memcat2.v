/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.will need a Picture Elements Binary Software
 *    License.
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
 * This program demonstrates the mixing of reg and memories in l-value
 * contatenations.
 */
module main;

   reg [3:0] mem [2:0];
   reg	     a, b;

   initial begin
      mem[0] = 0;
      mem[1] = 0;
      mem[2] = 0;

      {b, mem[1], a} = 6'b0_0000_1;
      if (a !== 1'b1) begin
	 $display("FAILED -- a = %b", a);
	 $finish;
      end
      if (mem[1] !== 4'b0000) begin
	 $display("FAILED -- mem[1] = %b", mem[1]);
	 $finish;
      end
      if (b !== 1'b0) begin
	 $display("FAILED -- b = %b", b);
	 $finish;
      end

      {b, mem[1], a} = 6'b0_1111_0;
      if (a !== 1'b0) begin
	 $display("FAILED -- a = %b", a);
	 $finish;
      end
      if (mem[0] !== 4'b0000) begin
	 $display("FAILED -- mem[0] - %b", mem[0]);
	 $finish;
      end
      if (mem[1] !== 4'b1111) begin
	 $display("FAILED -- mem[1] = %b", mem[1]);
	 $finish;
      end
      if (b !== 1'b0) begin
	 $display("FAILED -- b = %b", b);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
