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
//  SDW - Validate function called from within function

module main ();

reg [31:0] val1,val2 ;
reg error;

function [31:0] myfunc2 ;
   input [31:0] in2 ;
   myfunc2 = in2;
endfunction

function [31:0] myfunc1 ;
  input [31:0] in1 ;
  myfunc1 = myfunc2(in1) ;
endfunction

initial
  begin
    error = 0;
    val1 = myfunc1(32'h0) ;
    if(val1 != 32'h0)
      begin
        $display("FAILED - function3.11C - function called from funct(1)");
        error = 1;
      end

    val2 = 32'h12345678 ;
    val1 = myfunc1(val2);
    if(val1 != val2)
      begin
        $display("FAILED - function3.11C - function called from funct(2)");
        error = 1;
      end

    if(myfunc1(32'h10101010) != 32'h10101010)
      begin
        $display("FAILED - function3.11C - function called from funct(3)");
        error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule // main
