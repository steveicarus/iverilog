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
//  SDW - Check posedge vector - should use bit 0 only. Doesn't work with XL

module time2 ();

reg [3:0] clock;
reg [3:0] b;
reg	[3:0] count;

initial
  begin
    b = 4'b1111;
    count = 0;
    for (clock = 0; clock<=10; clock = clock + 1)
       begin
        $display("time = %t, clock = %h",$time,clock);
        #10;
       end
  end

always @(posedge clock & b)
  begin
    count = count+1;
    $display(" edge ! time = %t, count = %h",$time,count);
  end

initial
  begin
    #1000;
    if(count != 6)
        $display("FAILED - vect[0] clock detect count=%d",count);
    else
        $display("PASSED");
  end

endmodule
