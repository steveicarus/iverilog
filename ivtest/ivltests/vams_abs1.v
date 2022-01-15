/*
 *  Some more detailed tests of the abs() function.
 *
 *  Copyright (C) 2007-2008  Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

module main;

   reg signed [7:0] a, b;

   initial begin
      a = 1;
      b = -1;

      if (abs(0) !== 0) begin
	 $display("FAILED");
	 $finish;
      end

      if (abs(1) !== 1) begin
	 $display("FAILED");
	 $finish;
      end

      if (abs(-1) !== 1) begin
	 $display("FAILED");
	 $finish;
      end

      if (abs(a) !== 1) begin
	 $display("FAILED");
	 $finish;
      end

      if (abs(b) !== 1) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
