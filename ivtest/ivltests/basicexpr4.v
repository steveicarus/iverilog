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
// SDW: Synth of basic expression assign with add
//
//
module adder (q,a,b );
input a,b;
output [1:0] q;

assign q =  a  + b;

endmodule

module test ;

reg d;

wire [1:0] q;

adder u_add (.q(q),.a(d),.b(d));

(* ivl_synthesis_off *)
initial
  begin
//    $dumpfile("test.vcd");
//    $dumpvars(0,test);
    d = 0;
    # 1;
    if (q !== 2'b0)
       begin
         $display("FAILED - Q isn't 0 ");
	 $finish;
       end
    #1 ;
    d = 1;
    # 1;
    if (q !== 2'b10)
       begin
         $display("FAILED - Q isn't 2 ");
	 $finish;
       end
    $display("PASSED");

  end
endmodule
