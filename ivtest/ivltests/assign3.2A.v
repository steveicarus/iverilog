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
//  SDW - Validate assign identifier = expression ;

module main ;

wire a;
wire [31:0] b;
wire [15:0] c;

reg [31:0] val;
reg error;

assign a = val [0];		// Pull single bit
assign b = val;			// full variable
assign c = val[31:16];		// Top portion bit select

initial
  begin
    error = 0;
    if(a != 1'bx)
      begin
        $display("FAILED - 3.2A assign ident = expr");
         error = 1;
      end
    if(b != 32'bx)
      begin
        $display("FAILED - 3.2A assign ident = expr");
         error = 1;
      end
    if(c != 16'bx)
      begin
        $display("FAILED - 3.2A assign ident = expr");
         error = 1;
      end
    #1 ;
    val = 32'h87654321;
    #1 ;
    if(a != 1'b1)
      begin
        $display("FAILED - 3.2A assign ident = expr");
         error = 1;
      end
    if(b != 32'h87654321)
      begin
        $display("FAILED - 3.2A assign ident = expr");
         error = 1;
      end
    if(c != 16'h8765)
      begin
        $display("FAILED - 3.2A assign ident = expr");
         error = 1;
      end
    if(error == 0)
         $display("PASSED");

    $finish ;
  end


endmodule
