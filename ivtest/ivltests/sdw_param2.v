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
//
// SDW - Comma separated parameter def used as a width subscript
//
//
// D: This validates that parameters can be used as literals
// D: in the width subscript.
//

module main();

parameter VAL_1 = 5,
          VAL_2 = 0;

reg [VAL_1: VAL_2] temp_var;

initial   // Excitation block
  begin
    temp_var = 6'h11;
    #5 ;
  end

initial  // Validation block
  begin
    #1 ;
    if(temp_var != 6'h11)
      begin
        $display("FAILED - parameter assignment didn't work\n");
        $finish ;
      end


    $display("PASSED\n");
    $finish ;
  end

endmodule
