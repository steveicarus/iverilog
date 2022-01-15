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
 * Test the select of a bit from a vector.
 */

module main;

   reg  [3:0] a = 4'b0110;
   reg [1:0]  s = 0;

   wire       b = a[s];

   initial begin
      #1 if (b !== 0) begin
	 $display("FAILED -- a=%b, s=%b, b=%b", a, s, b);
	 $finish;
      end

      s = 1;

      #1 if (b !== 1) begin
	 $display("FAILED -- a=%b, s=%b, b=%b", a, s, b);
	 $finish;
      end

      s = 2;

      #1 if (b !== 1) begin
	 $display("FAILED -- a=%b, s=%b, b=%b", a, s, b);
	 $finish;
      end

      s = 3;

      #1 if (b !== 0) begin
	 $display("FAILED -- a=%b, s=%b, b=%b", a, s, b);
	 $finish;
      end

      s = 2'bxx;
      #1 if (b !== 1'bx) begin
	 $display("FAILED -- a=%b, s=%b, b=%b", a, s, b);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
