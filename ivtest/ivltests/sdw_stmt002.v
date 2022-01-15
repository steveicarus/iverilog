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
//    SDW - Rewrite of stmt002_bassign.v from vbs test suite.
//
module main;

reg [0:7] var1, var2;	// Note the obtuse bit ordering.
reg [3:0] var3;		// A more sane ordering on a nibble boundary...
reg var4;		// Single bit.
reg [2:9] var5;		// Use a non-alligned, reversed bit - still 8 bits
reg error;

initial
  begin
     // First verify that all the defined variables are x's.
     error = 0;
     if(var1 !== 8'hxx)
       begin
         $display("FAILED - sdw_stmt002 - var1 not 8'hxx");
          error = 1;
       end
     if(var2 !== 8'hxx)
       begin
         $display("FAILED -  sdw_stmt002 -var2 not 8'hxx");
          error = 1;
       end
     if(var3 !== 4'bx_xxx)
       begin
         $display("FAILED -  sdw_stmt002 -var3 not 4'hx");
          error = 1;
       end
     if(var4 !== 1'bx)
       begin
         $display("FAILED -  sdw_stmt002 -var4 not 1'bx");
          error = 1;
       end
     if(var5 !== 8'hxx)
       begin
         $display("FAILED -  sdw_stmt002 -var5 not 8'hxx");
          error = 1;
       end

     var1 = 8'b1001_0010;	// Do some binary bits
     var2 = 255;		// Fill it with decimal version of ff
     var3 = 4'hf;		// hex
     var4 = 0;
     var5 = 8'h99;		// Still 8 bits

     if(var1 != 8'h92)
       begin
         $display("FAILED - sdw_stmt002 - var1 not 8'h96");
          error = 1;
       end
     if(var2 != 8'hff)
       begin
         $display("FAILED -  sdw_stmt002 -var2 not 8'hff");
          error = 1;
       end
     if(var3 != 4'b1111)
       begin
         $display("FAILED -  sdw_stmt002 -var3 not 4'hf");
          error = 1;
       end
     if(var4 != 1'b0)
       begin
         $display("FAILED -  sdw_stmt002 -var4 not 1'b0");
          error = 1;
       end
     if(var5 != 8'h99)
       begin
         $display("FAILED -  sdw_stmt002 -var5 not 8'h99");
          error = 1;
       end

     // Next - assign sub-portion of vector
     var1 [3:6] = var3;

     if(var1 != 8'h9e)
       begin
         $display("FAILED -  sdw_stmt002 - subfield assign failed");
          error = 1;
       end

     var3 = 4'o11;	// Lets try octal now
     var4 = 1'b1;	// And set that bit to 1, it WAS 0
     var5 = 8'h66;	// Invert it

     if(var3 != 4'b1001)
       begin
         $display("FAILED -  sdw_stmt002 -var3 octal assign");
          error = 1;
       end
     if(var4 != 1'b1)
       begin
         $display("FAILED -  sdw_stmt002 -var4 not 1'b1");
          error = 1;
       end
     if(var5 != 8'h66)
       begin
         $display("FAILED -  sdw_stmt002 -var5 not 8'h66");
          error = 1;
       end

     // 9e, 9

     var3  = var1[4:7];		// Should be an 4'he
     var1[0:3] = var3[3:2];	// Now should give 8'hce

     if(var1 != 8'h3e)
       begin
         $display("FAILED - sdw_stmt002 - subfield assign(1) w/ 0 extension");
          error = 1;
       end
     if(var3 != 4'b1110)
       begin
         $display("FAILED -  sdw_stmt002 -subfield assign(2)");
          error = 1;
       end

     var3 = var5;		// 4 bit from 8 bit(4'h6)
     var5[5] = var4;		// Set var5 to 8'h76

     if(var3 != 4'h6)
       begin
         $display("FAILED -  sdw_stmt002 - 4bit from 8 bit assign");
          error = 1;
       end
     if(var5 != 8'h76)
       begin
         $display("FAILED -  sdw_stmt002 - single sub-bit assign ");
          error = 1;
       end

     if(error == 0)
       $display("PASSED");
  end

endmodule
