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
 * This program tests that a non-integer delay gets its extra
 * precision accounted for if the timescale supports it. In this
 * example, set the units to 1ms, but set the precision so that the
 * numbers can be given accurate to .1ms. This should cause a delay
 * of 2.4 and 2.6 to really be different.
 */

`timescale 1ms / 100us

module main;
   reg err;
   reg clk;
   reg out1, out2;
   realtime time1;
   realtime time2;

   real     eps;

   always @(posedge clk) #2.4 begin
      $display($time,,$realtime, " set out1 == 1");
      time1 = $realtime;
      out1 = 1;
   end

   always @(posedge clk) #2.6 begin
      $display($time,,$realtime, " set out2 == 1");
      time2 = $realtime;
      out2 = 1;
   end

   initial begin
      clk = 0;
      out1 = 0;
      out2 = 0;
      time1 = 0;
      time2 = 0;
      err = 0;
      $timeformat(-3,1,"ms",5);

      #1 if (out1 !== 0) begin
	 $display("Error -- out1 s/b 0 at time $time but is=%x", out1);
	 err =1 ;
      end

      clk = 1;

      #3 if (out1 !== 1) begin
	 $display("Error -- out1 s/b 1 at time $time but is=%x", out1);
	 err =1 ;
      end

      eps = time1 - 3.4;
      if (eps < 0.0)
	eps = 0.0 - eps;

      if (eps > 0.0001) begin
	 $display("Error -- time1 s/b 3.4 but is=%t", time1);

	 err =1 ;
      end

      #1 eps = time2 - 3.6;
      if (eps < 0.0)
	eps = 0.0 - eps;
      if (eps > 0.0001) begin
	 $display("Error -- time2 s/b 3.6 but is=%t, ", time2);
	 err =1 ;
      end

      if(err == 0)
         $display("PASSED");
      else
         $display("FAILED");
   end // initial begin

endmodule // main
