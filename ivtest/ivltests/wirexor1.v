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
//  SDW - Validate continuous xor in assignment..dependent on always ^ working
//


module main;

reg globvar;

reg [3:0] var1,var2,var3;
wire [3:0] var3a;
reg error;

assign var3a = var1 ^ var2;

always @( var1 or var2)
  var3 = var1 ^ var2 ;

initial
begin
error = 0;
for ( var1 = 4'b0; var1 != 4'hf; var1 = var1 + 1)
  for ( var2 = 4'b0; var2 != 4'hf; var2 = var2 + 1)
     begin
        #1 ;
        if(var3 != var3a)
         begin
           $display("FAILED continuous xor 1=%h,2=%h,3=%h,3a=%h",
                     var1,var2,var3,var3a);
           error = 1;
         end
        #1;
     end
if(error == 0)
  $display("PASSED");
end

endmodule // main
