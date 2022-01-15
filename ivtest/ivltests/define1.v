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
//  SDW - Play with defines a bit
//

`define NUM1 10
`define NUM2 4'b0001
`define NUM3 4'h4
`define WIDTH 4

module define1 ;

reg [`WIDTH-1:0] val ;
reg error;

initial
  begin
     error = 0;
     val = `NUM1 ;
     if(val !== 10)
       begin
         error = 1;
         $display("FAILED - define NUM1 10 didn't");
       end

     val = `NUM2 ;
     if(val !== 4'h1)
       begin
         error = 1;
         $display("FAILED - define NUM1 10 didn't");
       end
     val = `NUM3 ;
     if(val !== 4'b0100)
       begin
         error = 1;
         $display("FAILED - define NUM1 10 didn't");
       end
     if(error == 0)
       $display("PASSED");
  end

endmodule
