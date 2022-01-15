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
// SDW: function calling function validated
//
// D: Instantiate a 2 functions, with the 2nd calling
// D: the first. Validate the results are correct.
//

module main ();

reg [3:0] global_reg;
reg [3:0] result;

// Instantiate the function to be called
function [3:0] my_func_2;
input [3:0]  a;
begin
   my_func_2 = a;
end
endfunction

// This is the calling function
function [3:0] my_func_1 ;
input [3:0] a;
begin
    my_func_1 = my_func_2(a) + my_func_2(a); // So call it twice!
end
endfunction

initial
  begin
    global_reg = 2;
    result = my_func_1(global_reg);

    if(result != 4)
      begin
         $display("FAILED - function didn't function!\n");
         $finish ;
      end

    $display("PASSED\n");
    $finish ;
  end
endmodule
