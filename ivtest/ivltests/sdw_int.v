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
// SDW - Int type declaration/validation
//
// D: Assign a 16 bit vector to an int and observe the 0 extension.
// D: Assign a 32 bit vector to an int and observer same value.
// D: Add -1 + 1. Add 0 and -1...and observer correct values.

module main();

reg [15:0] a;
reg [31:0] b;

integer result;
integer int_a;
integer int_b;

initial   // Excitation block
  begin
   a = 0;
   b = 0;
   result = 0;
   # 5;			// Assign a shorter value
   a = 16'h1234;	// should see 0 extension in result
   result = a;

   # 5;			// Assign a 32  bit vector
   b = 32'h12345678 ;
   result = b;

   # 5;			// Validate sum basic integer arithmetic
   int_a = -1 ;		// pun intended!
   int_b = 1;
   result = int_a + int_b;

   # 5;
   int_a = 0;
   int_b = -1;
   result = int_a + int_b;

  end

initial  // Validation block
  begin
    #1 ;
    #5 ;
    if(result != 32'h00001234)
      begin
        $display("FAILED - Bit extend wrong\n");
        $finish ;
      end

    #5 ;
    if(result != 32'h12345678)
      begin
        $display("FAILED - 32 bit assign wrong\n");
        $finish ;
      end

    #5 ;
    if(result != 32'h00000000)
      begin
        $display("FAILED - -1 + 1 = %h\n",result);
        $finish ;
      end

    #5 ;
    if(result != 32'hffffffff)
      begin
        $display("FAILED - 0 - 1 = %h\n",result);
        $finish ;
      end
    $display("PASSED\n");
    $finish ;
  end

endmodule
