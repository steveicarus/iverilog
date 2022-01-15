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
 * This sample tests that the supply0 and supply1 nets take on
 * the proper initial value. This adds to the supply1 test some
 * constant drivers that could tickle constant propagation bugs.
 */

module test;

   supply0 gnd;
   supply1 vdd;

   // These should drop away as meaningless.
   assign  gnd = 1;
   assign  vdd = 0;

   initial begin #1
      if (gnd !== 0) begin
	 $display("FAILED -- gnd == %b", gnd);
	 $finish;
      end

      if (vdd !== 1) begin
	 $display("FAILED -- vdd == %b", vdd);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
