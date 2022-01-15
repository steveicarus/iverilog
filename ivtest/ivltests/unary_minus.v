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

module main;

   reg [7:0] x, y;

   initial begin
      x = -4;
      if (x !== 8'hfc) begin
	 $display("FAILED -- x = -4 --> %b", x);
	 $finish;
      end

      x = 4;
      if (x !== 8'h04) begin
	 $display("FAILED");
	 $finish;
      end

      y = -x;
      if (y !== 8'hfc) begin
	 $display("FAILED -- y = -%b --> %b", x, y);
	 $finish;
      end

      $display("PASSED");
   end // initial begin


endmodule // main
