//
// Copyright (c) 1999 Paul Bain (pdbain@adm.org)
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
//  SDW - PR122 - Const define without length specification causes error.

`timescale 1ns/1ns
module main(
clk,
dat
);

parameter dat_width =32;
input clk;
output [dat_width-1:0] dat;
reg [dat_width-1:0] dat;
reg [32-1:0] exp_dat;
reg error;

initial
  begin
    exp_dat = 0;
    dat = 0;
  end

initial
  begin
   dat = #1 'h00010203;
   exp_dat = #1 'h0010203;
   error = 0;
   #10;
   for (exp_dat = 0; exp_dat != 4'hf; exp_dat = exp_dat + 1)
     begin
       dat = exp_dat;
       #1
       if(dat !== exp_dat)
         begin
           $display("ERROR: dat = %h, exp_dat = %h",dat,exp_dat);
	   error = 1;
         end
       else
         $display("Okay: dat = %h, exp_dat = %h",dat,exp_dat);
     end
    if(error === 0)
       $display("PASSED");
    else
       $display("FAILED");
   end
endmodule
