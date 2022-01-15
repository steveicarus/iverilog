//
// Copyright (c) 2002 Steven Wilson (steve@ka6s.com)
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
// SDW: synth basic state machine form
//
//
module sm ( clk,rst,st);
input clk,rst;
output [1:0] st;

reg [1:0] st;

always @(posedge clk or posedge rst)
  if (rst)
     st <= 2'b0;
  else
     case (st)
       2'b00: st <= 2'b01;
       2'b01: st <= 2'b11;
       2'b11: st <= 2'b10;
       2'b10: st <= 2'b00;
     endcase
endmodule

module test ;

reg clk,rst;

wire [1:0] st;

sm u_sm ( clk,rst,st);

always #5 clk = ~clk;

initial
  begin
//    $dumpfile("test.vcd");
//    $dumpvars(0,test);
    clk = 0;
    rst = 1;
    @(posedge clk);
    #1 ;
    rst = 0;
    if(st !== 2'b00)
       begin
	 $display("FAILED - SM didn't initialize");
	 $finish;
       end
    @(posedge clk);
    #1 ;
    if(st !== 2'b01)
       begin
	 $display("FAILED - SM didn't xsn to 01");
	 $finish;
       end
    @(posedge clk);
    #1 ;
    if(st !== 2'b11)
       begin
	 $display("FAILED - SM didn't xsn to 11");
	 $finish;
       end
    @(posedge clk);
    #1 ;
    if(st !== 2'b10)
       begin
	 $display("FAILED - SM didn't xsn to 10");
	 $finish;
       end
    @(posedge clk);
    #1 ;
    if(st !== 2'b00)
       begin
	 $display("FAILED - SM didn't xsn to 00");
	 $finish;
       end
    $display("PASSED");
    $finish;
  end
endmodule
