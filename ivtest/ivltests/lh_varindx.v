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
//  SDW - Validate left hand variable index

module main ;

reg [3:0] a,b;
reg c;
reg error;

always @(c or b)
   a[b] = c;

initial
  begin
    #1 ;
    a = 4'b1111;
    error = 0;
    b = 1'b0;
    c = 1'b0;
    #1 ;
    if(a != 4'b1110)
      begin
        $display("FAILED - var index -  a = %b, [b] = %d, c=%b",a,b,c);
        error = 1;
      end
    #1 ;
    b = 1;
    #1 ;
    if(a != 4'b1100)
      begin
        $display("FAILED - var index -  a = %b, [b] = %d, c=%b",a,b,c);
        error = 1;
      end

    if(error == 0)
       $display("PASSED");
  end

endmodule
