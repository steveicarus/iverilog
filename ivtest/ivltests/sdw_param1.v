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
// SDW - Simple parameter declaration
//
// D: Declare a parameter value, then assign it to a variable.
// D: Check the value of the variable.
//

module main();

parameter VAL_1	= 16'h0001;
parameter VAL_2 = 16'h5432;

reg [15:0] test_var;

initial   // Excitation block
  begin
    test_var = VAL_1 ;
    #5 ;
    test_var = VAL_2 ;
    #5 ;
  end

initial  // Validation block
  begin

    #1 ;
    if(test_var != 16'h0001)
      begin
        $display("FAILED - param 1st assign didn't work\n");
        $finish ;
      end

    #5 ;
    if(test_var != 16'h5432)
      begin
        $display("FAILED - param 2nd assign didn't work\n");
        $finish ;
      end

    $display("PASSED\n");
    $finish ;
  end

endmodule
