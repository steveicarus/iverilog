//
// Copyright (c) 1999 Peter Monta (pmonta@imedia.com)
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

module main;
  reg clk,reset;
  wire [3:0] a,b;

  swap s(clk,reset,a,b);

  initial begin
    clk = 0;
    reset = 0;
    #1; reset = 1; #1; reset = 0;
    #1; clk = 1; #5; clk = 0;
    if (a===4'd6 && b===4'd5)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule

module swap(clk,reset,a,b);
  input clk,reset;
  output [3:0] a,b;
  reg [3:0] a,b;

  always @(posedge clk or posedge reset)
    if (reset) begin
      a <= #1 4'd5;
      b <= #1 4'd6;
    end else begin
      a <= #1 b;
      b <= #1 a;
    end
endmodule
