// Copyright (c) 1999 Steven Wilson (stevew@home.com)
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
//  SDW - Identifies a scheduling bug.  a1 should always follow a2, but it
//        seems IV has a race.  d is set ONLY after a clock occurs, yet
//        a1 sets as if the clock hasn't occured??

module test;

parameter p_dly = 0;
reg d,a1,a2,b2;
reg err;

reg clk;
wire b1,c;


always #5 clk = ~clk;

assign #p_dly c = d;
assign b1 = c;
always @(c)
	b2 = c;

always @(posedge clk)
  begin
   a1 <= b1;
   a2 <= b2;
  end

always @(negedge clk)
  if(a1 != a2)
     err = 1;

initial
  begin
//    $dumpfile("test.vcd");
//    $dumpvars(0,test);
    err = 0;
    clk = 0;
    d = 0;
    #20;
    @(posedge clk)
    d <= 1;
    #25;
    if(err == 1)
      $display("FAILED");
    else
      $display("PASSED");
    $finish;
  end
endmodule
