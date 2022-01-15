//
// Copyright (c) 1999 Steven Wilson (stevew@telocity.com)
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
//  SDW - Test non-constant bit selects - causes compile error right now

module test;

reg clk;
reg [1:0] in0;
reg [1:0] in1;
reg sel0,sel1;
wire [1:0] q;

dff2 u1 (q,clk,in0[sel0],in1[sel1]);

initial
  begin
     clk = 0;
     in0 = 2'b0;
     in1 = 2'b0;
     sel0 = 1'b0;
     sel1 = 1'b1;
     #8;
     $display("initial val =%x",q);
     #8;
     if(q == 2'b0)
	     $display("PASSED");
     else
	     $display("FAILED");
     $finish ;
  end

always #5 clk = ~clk;

endmodule


// This is just a dual dff
module dff2 (q,clk,d0,d1);

input clk,d0,d1;
output [1:0] q;

reg [1:0] q;

  always @(posedge clk)
	  q <= {d1,d0};

endmodule
