/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
 * Test the ability to resolve resolve tri-state drivers onto a single
 * signal. Use multiple continuous assignments to a wire to create
 * multiple drivers, and use the ?: operator to tri-state the driven
 * value based on the sel value.
 */

module main;

   wire [1:0] out;

   reg [1:0] sel = 2'bzz;
   reg [1:0] v0 = 0;
   reg [1:0] v1 = 1;
   reg [1:0] v2 = 2;
   reg [1:0] v3 = 3;

   assign    out = (sel == 2'b00)? v0 : 2'bz;
   assign    out = (sel == 2'b01)? v1 : 2'bz;
   assign    out = (sel == 2'b10)? v2 : 2'bz;
   assign    out = (sel == 2'b11)? v3 : 2'bz;

   initial begin

      #1 if (out !== 2'bxx) begin
	 $display("FAILED -- sel==%b, out==%b", sel, out);
	 $finish;
      end

      sel = 0;
      #1 if (out !== 2'b00) begin
	 $display("FAILED -- sel==%b, out==%b, v0==%b", sel, out, v0);
	 $finish;
      end

      sel = 1;
      #1 if (out !== 2'b01) begin
	 $display("FAILED -- sel==%b, out==%b", sel, out);
	 $finish;
      end

      sel = 2;
      #1 if (out !== 2'b10) begin
	 $display("FAILED -- sel==%b, out==%b", sel, out);
	 $finish;
      end

      sel = 3;
      #1 if (out !== 2'b11) begin
	 $display("FAILED -- sel==%b, out==%b", sel, out);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
