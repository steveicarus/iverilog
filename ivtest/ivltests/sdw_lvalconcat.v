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
// SDW - Validate that an lvalue concat can receive an assignment.
//
// D: Validate that an lvalue can be a concatenation.
//

module main ();
reg a;
reg b;
reg working;

initial
begin
    working = 1;
    {a,b} = 2'b00 ;

    if( (a != 0) & (b != 0))
       begin
         $display("FAILED {a,b} Expected 2'b00 - received %b%b",a,b);
          working = 0;
       end

    {a,b} = 2'b01 ;

    if( (a != 0) & (b != 1))
       begin
         $display("FAILED {a,b} Expected 2'b01 - received %b%b",a,b);
         working = 0;
       end

    {a,b} = 2'b10 ;

    if( (a != 1) & (b != 0))
       begin
         $display("FAILED {a,b} Expected 2'b10 - received %b%b",a,b);
         working = 0;
       end

    {a,b} = 2'b11 ;

    if( (a != 1) & (b != 1))
       begin
         $display("FAILED {a,b} Expected 2'b11 - received %b%b",a,b);
         working = 0;
       end

    if(working)
       $display("PASSED\n");

end

endmodule
