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
// SDW: Function scope handling
//
// D: Validate scope handling of variables
//

module main ();

reg [3:0] global_reg;
reg [3:0] result;

function [3:0] my_func ;
input [3:0] a;
reg [3:0] global_reg;
begin
    global_reg = a + a;
    my_func = a + a;
end
endfunction

initial
  begin
    global_reg = 2;
    result = my_func(global_reg);

    if(result != 4)
      begin
         $display("FAILED - function didn't function!\n");
         $finish ;
      end

    if(global_reg != 2)
      begin
         $display("FAILED - function scope problem!\n");
         $finish ;
      end

    $display("PASSED\n");
    $finish ;
  end
endmodule
