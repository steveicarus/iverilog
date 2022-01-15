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
//  SDW - Validate various module formats


// Template 1

module mod1 ;
endmodule

// Template 2
module mod2 ();
endmodule

// Template 3
module mod3 (a,b);
input a;
input b;
endmodule

//Template 4  -
module mod4 (ident1,out1);
input  [31:0]  ident1;
output [31:0]  out1;

wire [31:0] out1 = ident1;
endmodule

module main ();

wire [31:0] out1,out2;
reg  [31:0] val1,val2;
reg error;

mod4 inst1 (val1,out1);			// Ordered port list
mod4 inst2 (.ident1(val2),.out1(out2)); // List by portname

initial
 begin
    error = 0;
    val1 = 32'h11223344;
    #1 if(out1 != 32'h11223344)
      begin
        $display("FAILED - module 3.12A - Ordered module port list failed");
        error = 1;
      end
    val2 = 32'h44332211;
    #1 if(out2 != 32'h44332211)
      begin
        $display("FAILED -module 3.12A -named module port list (.x(a)) failed");
        error = 1;
      end
    if(error == 0)
      $display("PASSED");
 end


endmodule // main
