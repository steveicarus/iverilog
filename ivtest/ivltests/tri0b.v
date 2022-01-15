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

/*
 * This module tests the basic behavior of a tri0 register. We use a ?:
 * to turn on/off the driver to the tri0 net and watch its value.
 * A tri0 net should pull to 0 when undriven, and follow the driver
 * otherwise.
 */
module main;

   reg enable, val;
   tri0 t0 = (~enable) ? 1'bz : val;

   initial begin
      enable = 0;
      val = 0;

      #1 if (t0 !== 1'b0) begin
	 $display("FAILED -- undriven t0 == %b", t0);
	 $finish;
      end

      enable = 1;

      #1 if (t0 !== 1'b0) begin
	 $display("FAILED -- driven-0 t0 == %b", t0);
	 $finish;
      end

      val = 1;

      #1 if (t0 !== 1'b1) begin
	 $display("FAILED -- driven-1 t0 == %b", t0);
	 $finish;
      end

      $display("PASSED");
   end // initial begin
endmodule // main
