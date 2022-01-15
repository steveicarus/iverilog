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
 * This module checks that parameter bit select works
 * in parameter assignment expressions.
 */
module main;

   parameter value = 2'b10;
   parameter x = 0;
   parameter y = 1;
   parameter pa = value[0];
   parameter pb = value[1];
   parameter px = value[x];
   parameter py = value[y];

   initial begin

      if (pa !== value[0]) begin
	 $display("FAILED -- pa == %b", pa);
	 $finish;
      end

      if (pa !== 0) begin
	 $display("FAILED -- pa == %b", pa);
	 $finish;
      end

      if (pb !== value[1]) begin
	 $display("FAILED -- pb == %b", pb);
	 $finish;
      end

      if (pb !== 1) begin
	 $display("FAILED -- pb == %b", pb);
	 $finish;
      end

      if (px !== value[0]) begin
	 $display("FAILED -- px == %b", px);
	 $finish;
      end

      if (px !== 0) begin
	 $display("FAILED -- px == %b", px);
	 $finish;
      end

      if (py !== value[1]) begin
	 $display("FAILED -- py == %b", py);
	 $finish;
      end

      if (py !== 1) begin
	 $display("FAILED -- py == %b", py);
	 $finish;
      end
      $display("PASSED");
   end // initial begin

endmodule // main
