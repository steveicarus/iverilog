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
 * This program tests that a non-integer delay, in the absence of any
 * timescale values, will round properly. A 2.4 delay rounds to 2, and
 * a 2.6 delay rounds to 3.
 */

module main;

   reg clk;
   reg out1, out2;
   time time1;
   time time2;

   always @(posedge clk) #2.4 begin
      $display($time,, "set out1 == 1");
      time1 = $time;
      out1 = 1;
   end

   always @(posedge clk) #2.6 begin
      $display($time,, "set out2 == 1");
      time2 = $time;
      out2 = 1;
   end

   initial begin
      clk = 0;
      out1 = 0;
      out2 = 0;
      time1 = 0;
      time2 = 0;

      #1 if (out1 !== 0) begin
	 $display("FAILED -- out1 is not 0: %b", out1);
	 $finish;
      end

      clk = 1;

      #3 if (out1 !== 1) begin
	 $display("FAILED -- out is not 1 at time 3: %b", out1);
	 $finish;
      end

      if (time1 != 3) begin
	 $display("FAILED -- time1 = %d", time1);
	 $finish;
      end

      #1 if (time2 != 4) begin
	 $display("FAILED -- time2 = %d", time2);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
