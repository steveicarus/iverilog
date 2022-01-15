//
// Copyright (c) 2002 Steven Wilson (steve@ka6s.com)
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
// SDW: Synth of basic latch form
//
//
module basiclatch ( clk, d, q);
input clk, d;
output q;
reg q;

always @ (clk or d)
  if(~clk)
    q = d;

endmodule

module tbench ;

reg clk, d;

basiclatch u_reg (clk,d,q);

initial
  begin
    clk = 0;
    d = 0;
    #1 ;
    if(q !== 0)
       begin
         $display("FAILED - initial value not 0");
	 $finish;
       end
    #1 ;
    clk = 1;
    # 1;
    d = 1;
    # 1;
    if(q !== 0)
       begin
         $display("FAILED - Didn't latch initial 0");
	 $finish;
       end
    #1
    clk = 0;
    # 1;
    if(q !== 1)
       begin
         $display("FAILED - Didn't pass 1 after latch dropped");
	 $finish;
       end
    #1
    clk = 1;
    # 1;
    d = 0;
    # 1;
    if(q !== 1)
       begin
         $display("FAILED - Didn't latch 1 after latch dropped");
	 $finish;
       end

    $display("PASSED");
  end
endmodule
