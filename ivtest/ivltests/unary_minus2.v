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


`timescale  1ns /  1ns

module U1 (OUT);

   parameter VALUE = -384;

   output [9:0] OUT;

   assign	OUT = VALUE;

endmodule

module U2 (OUT);

   parameter VALUE = 96;

   output [9:0] OUT;

   assign	OUT = VALUE;

endmodule

module main;
   wire [9:0] out1, out2;

   U1 u1 (out1);
   U2 u2 (out2);

   initial #1 begin
      if (out1 !== 10'h280) begin
	 $display("FAILED -- out1 = %b", out1);
	 $finish;
      end

      if (out2 !== 10'h060) begin
	 $display("FAILED -- out2 = %b", out2);
	 $finish;
      end

      $display("PASSED");
   end // initial #1
endmodule // main
