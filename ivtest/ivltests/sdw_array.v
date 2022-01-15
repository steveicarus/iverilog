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
// SDW - Memory array instantiation, validation
//
// D: First do the declaration, and assignment of bit wide arrays
// D: and 16 bit wide 4 deep arrays.  Then assign values and validate
// D: the assignment worked.

module main();

reg		mem_1 [1:0];	// Define 2 locations, each 1 bit in depth
reg [15:0]	mem_2 [3:0];	// Define a 16 bit wide array - 4 in depth
reg [15:0]	work16;
reg		work1;

initial   // Excitation block
  begin
	mem_1 [0] = 0;		// Do the initial assignment of values
	mem_1 [1] = 1;
	mem_2 [0] = 16'h0;
	mem_2 [1] = 16'h1;
	mem_2 [2] = 16'h2;
	mem_2 [3] = 16'h3;

        #5 ;
        mem_1 [1] = mem_1 [0] ;	// use the mem array on the rhs
        mem_2 [3] = mem_2 [0] ;

        #5;

  end

initial  // Validation block
  begin
    #1 ;
    // Validate initialization
    work1 = mem_1[0];
    if(work1 != 0)
       begin
         $display("FAILED - mem_1 [0] init failed\n");
         $finish ;
       end
    work1 = mem_1[1];
    if(work1 != 1)
       begin
         $display("FAILED - mem_1 [1] init failed\n");
         $finish ;
       end
    work16 = mem_2 [0];
    if(work16 != 16'h0)
       begin
         $display("FAILED - mem_2 [0] init failed\n");
         $finish ;
       end
    work16 = mem_2 [1];
    if(work16 != 16'h1)
       begin
         $display("FAILED - mem_2 [1] init failed\n");
         $finish ;
       end
    work16 = mem_2 [2];
    if(work16 != 16'h2)
       begin
         $display("FAILED - mem_2 [2] init failed\n");
         $finish ;
       end
    work16 = mem_2 [3];
    if(work16 != 16'h3)
       begin
         $display("FAILED - mem_2 [3] init failed\n");
         $finish ;
       end

    #5 ;
    work1 = mem_1[1];
    if(work1 != 0)
       begin
         $display("FAILED - mem_1 [1] rhs assignment \n");
         $finish ;
       end
    work16 = mem_2 [3];
    if(work16 != 16'h0)
       begin
         $display("FAILED - mem_2 [3] rhs assignment\n");
         $finish ;
       end

    $display("PASSED\n");
    $finish ;
  end

endmodule
