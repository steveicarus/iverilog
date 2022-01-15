//
// Copyright (c) 2001 Steve Wilson (stevew@home.com)
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
//  SDW - From PR 272 reported by Lennart Augustsson <augustss@augustsson.net>

module test;

  reg  clk;
  reg [31:0] x;
  reg [31:0] y;
  reg error;

  always@(posedge clk)
        x <= -y;

  always #2 clk = ~clk;

  initial
    begin
       clk = 0;
       error = 0;
       y = 0;

       #10;
       if( x !== 32'h0)
         begin
           error = 1;
	   $display("FAILED - X should still be 0, and it's not");
         end

       #10;
       y = 32'h11111111;
       #10;
       if(x !== 32'heeee_eeef)
         begin
           error = 1;
	   $display("FAILED - X should still be EEEE_EEEF, rather x=%h",x);
	 end

       #10;
       if(error == 0)
	  $display("PASSED");

       $finish ;
    end

endmodule
