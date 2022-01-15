// Copyright (c) 2000 Steve Wilson (stevew@home.com)
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

// SDW:  Try the always @ * construct from Verilog 2000 LRM spec

module test;


//
// Define a procedural assignment based mux.
//
reg [1:0] sel;
reg [1:0] out, a,b,c,d;
reg error;

always @ *
   case (sel)
       2'b00: out = a;
       2'b01: out = b;
       2'b10: out = c;
       2'b11: out = d;
   endcase

initial
  begin
    error  = 0;
    #1 ;
    sel = 0;
    a = 0;
    #1;
    if(out !== 2'b00)
        begin
          $display("FAILED - Wildcard sensitivy list a != 0(1)");
          error =1;
        end
     #1;
     a = 1;
     #1;
     if(out !== 2'b01)
        begin
          $display("FAILED - Wildcard sensitivity list a != 1 (2)");
          error =1;
        end
     if(error == 0)
       $display("PASSED");
  end

endmodule
