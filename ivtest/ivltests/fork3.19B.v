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
//  SDW - Validate fork template 2

module main ;

reg [3:0] value1,value2 ;
reg [3:0] ret1,ret2 ;
reg error;

always @(value1 or value2)
  begin
    fork
       #10 ret1 = value1;
       @(ret1) #12 ret2 = value2;
    join
  end

initial
  begin
    error = 0;
    #1;
    value1 = 1;
    value2 = 2;
    ret1 = 0;
    ret2 = 0;
    #12;
    if(ret1 !== 1)
       begin
         $display("FAILED - force3.19B first statement didn't execute(1)");
         error = 1;
       end
    if(ret2 !== 0)
       begin
         $display("FAILED - force3.19B second stmt executed? is %d sb %d",
                   1'b0,ret2);
         error = 1;
       end
     #10;
    if(ret1 !== 1)
       begin
         $display("FAILED -fork3.19B First statement problem sb 1, is %d",ret1);
         error = 1;
       end
    if(ret2 !== 2)
       begin
         $display("FAILED -fork3.19B First statement problem sb 2, is %d",ret1);
         error = 1;
       end

     if(error == 0)
       $display("PASSED");
  end

endmodule
