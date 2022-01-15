/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
 * test the signedness of wires.
 */
module main;

   reg signed [7:0] val_rs = -5;
   wire [7:0] val_w = val_rs + 1;
   wire       signed [7:0] val_ws = val_rs + 1;

   initial begin
      #1 /* Let assignments settle. */
	$display("val_w=%d, val_ws=%d", val_w, val_ws);

      if (val_w !== 8'd252) begin
	 $display("FAILED -- val_w is wrong: %b", val_w);
	 $finish;
      end

      if (val_ws !== -8'sd4) begin
	 $display("FAILED == val_ws is wrong: %b", val_ws);
	 $finish;
      end

      if (val_ws > 0) begin
	 $display("FAILED -- signed test of val_ws failed");
	 $finish;
      end

      if (val_w < 0) begin
	 $display("FAILED -- signed test of val_w failed");
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
