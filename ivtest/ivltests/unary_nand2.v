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
//  SDW - Validate unary nand ~&(value)
//

// SJW - Make a version that uses behavioral code to implement ~&

module main;

reg [3:0] vect;
reg	error;
reg	result;


initial
 begin
   error = 0;
   for(vect=4'b000;vect<4'b1111;vect = vect + 1)
    begin
      result = ~& vect;
      if(result !== 1'b1)
        begin
           $display("FAILED - Unary nand ~&(%b)=%b",vect,result);
           error = 1'b1;
        end
    end
   #1;
   vect = 4'b1111;
   result = ~& vect;
   if(result !== 1'b0)
     begin
       $display("FAILED - Unary nand ~&(%b)=%b",vect,result);
       error = 1'b1;
     end
   if(error === 0 )
     $display("PASSED");
 end

endmodule // main
