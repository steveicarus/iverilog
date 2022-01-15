/*
 * Copyright (c) 2001 Peter Bain
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

/* This is based on PR#124. */

`timescale 1ns/1ns
module paramtest(clk, dat);
   parameter dat_width = 32;
   input     clk;
   output [dat_width-1:0] dat;

   reg [dat_width-1:0]	  dat;
   reg [4-1:0]		  exp_dat;
   parameter		  pay_init = 32'h01020304;
   parameter		  pay_inc = 32'h01010101;

   parameter		  cell_size = (53 * 8);
   parameter		  transfers = cell_size/dat_width + ((cell_size%dat_width)?1:0);

   initial begin
      exp_dat = 0;
      dat = 0;
   end


   initial begin
      #10;
      for (exp_dat = 0; exp_dat != 4'hf; exp_dat = exp_dat + 1) begin
         dat <= exp_dat;
         #1
           if (dat !== exp_dat) begin
              $display("ERROR: dat = %h, exp_dat = %h", dat, exp_dat);
	   end else begin
              $display("OKAY: dat = %h, exp_dat = %h", dat, exp_dat);
           end
      end
   end
endmodule
