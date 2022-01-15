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
 * This test tests simple zero-extend of small r-values into large
 * l-values.
 *
 * Correction and extensions by EML; 2007-11-15
 */
module main;

   reg [3:0] y;
   reg signed xs;
   reg	      x;
   reg	      fail;

   initial begin
      fail = 0;
      x = 1'b0;
      y = x;
      if (y !== 4'b0000) begin
	 $display("FAILED 1 -- x=%b, y=%b", x, y);
	 fail = 1;
      end

      x = 1'b1;
      y = x;
      if (y !== 4'b0001) begin
	 $display("FAILED 2 -- x=%b, y=%b", x, y);
	 fail = 1;
      end

      // x is a 1-bit unsigned reg; it zero-extends when assigned to y
      x = 1'bx;
      y = x;
      if (y !== 4'b000x) begin
	 $display("FAILED 3 -- x=%b, y=%b", x, y);
	 fail = 1;
      end

      // x is a 1-bit unsigned reg; it zero-extends when assigned to y
      x = 1'bz;
      y = x;
      if (y !== 4'b000z) begin
	 $display("FAILED 4 -- x=%b, y=%b", x, y);
	 fail = 1;
      end

      // xs is a 1-bit signed reg; it top-bit-extends when assigned to y
      xs = 1'bx;
      y = xs;
      if (y !== 4'bxxxx) begin
	 $display("FAILED 5 -- xs=%b, y=%b", xs, y);
	 fail = 1;
      end

      // xs is a 1-bit signed reg; it top-bit-extends when assigned to y
      xs = 1'bz;
      y = xs;
      if (y !== 4'bzzzz) begin
	 $display("FAILED 6 -- xs=%b, y=%b", xs, y);
	 fail = 1;
      end

      // 'bx is an unsized unsigned constant; it X-extends to the size of
      // the expression it is in
      y = 'bx;
      if (y !== 4'bxxxx) begin
	 $display("FAILED 7 -- y=%b", y);
	 fail = 1;
      end

      // 'bz is an unsized unsigned constant; it Z-extends to the size of
      // the expression it is in
      y = 'bz;
      if (y !== 4'bzzzz) begin
	 $display("FAILED 8 -- y=%b", y);
	 fail = 1;
      end

      // this is the only case in which a constant pads to the left with
      // X's. 4'bx is 4-bit unsigned, but it is specified with fewer than 4
      // bits
      y = 4'bx;
      if (y !== 4'bxxxx) begin
	 $display("FAILED 9 -- y=%b", y);
	 fail = 1;
      end

      // this is the only case in which a constant pads to the left with
      // Z's. 4'bz is 4-bit unsigned, but it is specified with fewer than 4
      // bits
      y = 4'bz;
      if (y !== 4'bzzzz) begin
	 $display("FAILED 10 -- y=%b", y);
	 fail = 1;
      end

      if (!fail) $display("PASSED");
   end // initial begin

endmodule // main
