//
// Copyright (c) 1999 Peter Monta (pmonta@imedia.com)
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
//  9/7/99 - SDW - Modified by instantiating Peter's module into a
//                 self-checking structure. Moved bar=result inside
//                 begin clause in function.
//
//  SDW - Validate function contains a register

module main ();

reg [1:0] val1;
reg	  val2;
reg	  error;

function bar;
    input [1:0] arg;
    reg result;
    begin
      result = |arg;
      bar = result;
    end
endfunction

initial
  begin
    error = 0;
    val2  = bar(2'b01);
    if(val2 != 1)
      begin
        $display("FAILED function 3.11F - register within a function(1)");
        error = 1;
      end
    val1 = 2'b11 ;
    val2 = bar(val1) ;
    if(val2 != 1)
      begin
        $display("FAILED function 3.11F - register within a function(2)");
        error = 1;
      end
    val2 = bar(2'b00);
    if(val2 != 0)
      begin
        $display("FAILED function 3.11F - register within a function(2)");
        error = 1;
      end
    if(error == 0)
      $display("PASSED");
  end

endmodule // main
