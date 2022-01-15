/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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

module main;

   reg [7:0] a;
   reg [6:-1] b;

   initial begin
      a = 1;
      b = 2;
      if (a !== 8'h01) begin
	 $display("FAILED -- to initialize a: %b", a);
	 $finish;
      end

      if (b !== 8'h02) begin
	 $display("FAILED -- to initialize b: %b", b);
	 $finish;
      end

      b = a;
      if (b !== 8'h01) begin
	 $display("FAILED -- to copy a to b: %b", b);
	 $finish;
      end

      $display("PASSED");
   end // initial begin
endmodule // main
