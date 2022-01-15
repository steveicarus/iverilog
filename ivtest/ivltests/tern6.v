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
 * This program catches some glitches in the MUXZ that Icarus Verilog
 * uses to implement the ?: in structural cases.
 */
module main;

   reg [6:0] a, b;
   reg	     sel;

   wire [6:0] test = sel? a : b;

   wire [7:0] test2 = test;

   initial begin
      sel = 0;
      // At this point, test2 should be x.
      #1 $display("sel=%b, test2=%b", sel, test2);

      b = 0;
      #1 $display("sel=b, test2=%b", sel, test2);
      if (test2 !== 8'b0_0000000) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
