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
//  SDW - Validate function within a continuous assignment

module main ();

reg error;
reg [3:0] val2;


function [3:0] myfunc ;
  input [31:0] in1 ;
  myfunc = in1;
endfunction

wire [3:0] val1;
assign val1 = myfunc(val2);

initial
  begin
    error = 0;
    val2 = 4'h0;
    # 1 ;
    if(val1 !== 4'b0)
      begin
        $display("FAILED - function3.11D - function within continuous assign(1)");
        error = 1;
      end

    val2 = 32'h8;
    # 1 ;
    if(val1 !== val2)
      begin
        $display("FAILED - function3.11D - function within continuous assign(2)");
        error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule // main
