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
// SDW: Synth of basic reg form
//
//
module basicreg ( clk, d, q);
input clk, d;
output q;
reg q;

(* ivl_synthesis_on *)
always @(posedge clk)
	q <= d;

endmodule

module tbench ;

reg clk, d;

basicreg u_reg (clk,d,q);

(* ivl_synthesis_off *)
initial
  begin
    clk = 0;
    d = 0;
    # 1;
    clk = 1;
    # 1;
    if (q !== 0)
       begin
         $display("FAILED - Q isn't 0 on first edge");
	 $finish;
       end
    d = 1;
    # 1;
    clk = 0;
    # 1;
    if (q !== 0)
       begin
         $display("FAILED - Q isn't 0 after first falling edge");
	 $finish;
       end
    # 1;
    d = 1;
    clk = 1;
    # 1;
    if (q !== 1)
       begin
         $display("FAILED - Q isn't 1 2nd raising edge");
	 $finish;
       end
    # 1;
    clk = 0;
    # 1;
    if (q !== 1)
       begin
         $display("FAILED - Q isn't 1 after 2nd falling edge");
	 $finish;
       end
    $display("PASSED");

  end
endmodule
