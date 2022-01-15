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
//  SDW - Validate inout with id(a,a) inout a; definition.

module id(a,a);
inout a;
endmodule

module top ();

wire b,c;

id i1(b,c);

reg a, error, ena_a,ena_b;

assign c = ena_a ? a : 1'bz;
assign b = ena_b ? a : 1'bz;

initial
  begin
     error = 0;
     ena_a = 1'b0;
     ena_b = 1'b0;
     #1 ;
     ena_a = 1'b1;
     #1 ;
     a= 0;
     #1;
     if(b !== 1'b0)
       begin
         error = 1;
         $display("FAILED - b init value not 1'b0 a=%b,b=%b,c=%b",a,b,c);
       end
     if(c !== 1'b0)
       begin
         error = 1;
         $display("FAILED - c init value not 1'b0  a=%b,b=%b,c=%b",a,b,c);
       end
     #1 ;
     a= 1;
     #1;
     if(b !== 1'b1)
       begin
         error = 1;
         $display("FAILED - b init value not 1'b1  a=%b,b=%b,c=%b",a,b,c);
       end
     if(c !== 1'b1)
       begin
         error = 1;
         $display("FAILED - c init value not 1'b1  a=%b,b=%b,c=%b",a,b,c);
       end
     #1 ;
     ena_a = 1'b0;
     #1 ;
     ena_b = 1'b1;
     #1 ;
     a= 0;
     #1;
     if(b !== 1'b0)
       begin
         error = 1;
         $display("FAILED - b init value not 1'b0 a=%b,b=%b,c=%b",a,b,c);
       end
     if(c !== 1'b0)
       begin
         error = 1;
         $display("FAILED - c init value not 1'b0  a=%b,b=%b,c=%b",a,b,c);
       end
     #1 ;
     a= 1;
     #1;
     if(b !== 1'b1)
       begin
         error = 1;
         $display("FAILED - b init value not 1'b1  a=%b,b=%b,c=%b",a,b,c);
       end
     if(c !== 1'b1)
       begin
         error = 1;
         $display("FAILED - c init value not 1'b1  a=%b,b=%b,c=%b",a,b,c);
       end


    if(error == 0)
       $display("PASSED");
  end

endmodule
