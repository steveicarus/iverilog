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
// SDW - Validate a lval concat in a continuous assignment
//
// D: Validate that an lvalue can be a concatenation.
//

module main ();
wire a, b;
reg [1:0] c;
reg working;

assign {a,b} = c;

initial
begin
    working = 1;
    c = 2'b00 ;

    #1 if( (a !== 0) & (b !== 0))
       begin
         $display("FAILED - {a,b} Expected 2'b00 - received %b%b",a,b);
          working = 0;
       end

    c = 2'b01 ;

    #1 if( (a !== 0) & (b !== 1))
       begin
         $display("FAILED {a,b} Expected 2'b01 - received %b%b",a,b);
         working = 0;
       end

    c = 2'b10 ;

    #1 if( (a !== 1) & (b !== 0))
       begin
         $display("FAILED {a,b} Expected 2'b10 - received %b%b",a,b);
         working = 0;
       end

    c = 2'b11 ;

    #1 if( (a !== 1) & (b !== 1))
       begin
         $display("FAILED {a,b} Expected 2'b11 - received %b%b",a,b);
         working = 0;
       end

    #1 if(working)
       $display("PASSED\n");

end

endmodule
