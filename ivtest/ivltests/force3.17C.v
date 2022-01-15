//
// Copyright (c) 2000 Steve Wilson (stevew@home.com)
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
//    force3.17A -  Template 1 - force net_lvalue = constant.
//

module test ;

reg [3:0] val1;
reg [3:0] val2;
wire [3:0] val3;

initial
  begin
   #50 ;
   if(val3 !== 4'b1010)
     $display("FAILED");
   else
     $display("PASSED");
  end

initial
  begin
   #20;
   force val3 = 4'b1010;
  end
endmodule
