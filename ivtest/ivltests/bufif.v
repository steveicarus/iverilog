// Copyright (c) 2000 Stephen Williams (steve@icarus.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//


/*
 * This module implements what essentially amounts to an array of DFF
 * devices with output enable. This test checks the operation of the
 * bufif0 and bufif1 devices.
 */
module grayGap (ad, clk, read, write);

   output [31:0] ad;
   input	 clk, read, write;

   reg [15:0] regff;

   bufif0 ad_drv [31:0] (ad, {16'b0, regff}, read);

   always @(posedge clk)
     if (write) regff = ad[15:0];


endmodule


module main;

   wire [31:0] ad;
   reg	       clk, read, write;

   reg [31:0]  ad_val;
   reg ad_en;

   bufif1 ad_drv[31:0] (ad, ad_val, ad_en);

   grayGap test (ad, clk, read, write);

   always #10 clk = ~clk;

   initial begin
      clk = 1;
      read = 1;
      write = 0;
      $monitor($time, "ad=%b", ad);

      // Set up to write a value into the grayGap register.
      @(negedge clk)
	ad_val = 32'haaaa_aaaa;
        read   = 1;
        write  = 1;
        ad_en  = 1;

      // The posedge has passed, now set up to read that value
      // out. Turn all the drivers off for a moment, to see that the
      // line becomes tri-state...
      @(negedge clk)
	ad_en = 0;
        write = 0;

      // Now read the value.
      #1 read = 0;

      #1 $display("Wrote %h, got %h", ad_val, ad);

      if (ad !== 32'b0000_0000_0000_0000_1010_1010_1010_1010) begin
	 $display("FAILED -- ad is %b", ad);
	 $finish;
      end

      #2 read = 1;

      $display("PASSED");
      $finish;
   end

endmodule // main
