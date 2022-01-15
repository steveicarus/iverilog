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
// SDW: Function with if clause
//
// D:
//

module main ();

reg [3:0] global_var;
reg [3:0] result;
// Interesting because 2 * 0 is  0 ;-)
function [3:0] my_func ;
input [3:0] a;
begin
   if(a == 4'b0)
      my_func = 4'b0;
   else
      my_func = a + a;
end
endfunction

initial
  begin
    global_var = 2;
    result = my_func(global_var);

    if(result != 4)
      begin
         $display("FAILED - function didn't function!\n");
         $finish ;
      end

    $display("PASSED\n");
    $finish ;
  end
endmodule
