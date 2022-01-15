//
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
//
// SDW - time3.v - Verify glitch doesn't propagate with non-blocking
//
// D:

module main ();

reg a;
reg error;
wire c;
reg d;

assign c = a;
always @(c)
  #5 d <= c;



always @(posedge d)
     error <= 1'b1;

initial
  begin
/*
    $dumpfile("/root/testsuite/dump.vcd");
    $dumpvars(0,main);
    $dumpon;
*/
    a =1'b0;
    error = 1'b0;
    #10;

    a = 1'b1;
    # 3;
    a = 1'b0;
    # 10;

    if(error)
       $display("FAILED");
    else
       $display("PASSED");
    #5;
    $finish;
  end


endmodule
