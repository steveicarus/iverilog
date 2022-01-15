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

module main;

   reg [1:0] out;

   initial begin
      out = 2'b0;

      if (out !== 2'b0) begin
	 $display("FAILED to initialize:  out == %b", out);
	 $finish;
      end

      out <= #5 2'b1;

      if (out !== 2'b0) begin
	 $display("FAILED -- changed immediately: out == %b", out);
	 $finish;
      end

      #4 if (out !== 2'b0) begin
	 $display("FAILED -- changed too soon: out == %b", out);
	 $finish;
      end

      #2 if (out !== 2'b1) begin
	 $display("FAILED to change after delay: out == %b", out);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
