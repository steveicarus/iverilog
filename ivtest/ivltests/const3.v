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

module main;

   reg [7:0] testr;
   wire [7:0] testw = {-5'd1, -3'sd1};

   initial begin
      #1 testr = {-5'd1, -3'sd1};
      if (testr !== 8'b11111_111) begin
	 $display("FAILED -- testr=%b", testr);
	 $finish;
      end

      if (testw !== 8'b11111_111) begin
	 $display("FAILED -- testw=%b", testw);
	 $finish;
      end

      $display("testr=%b", testr);
      $display("testw=%b", testw);
      $display("PASSED");
   end // initial begin

endmodule // main
