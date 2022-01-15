/*
 * Copyright (c) 2000 Peter monta (pmonta@pacbell.net)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
module main;

  function [3:0] foo;
    input [3:0] x;
    begin
      foo = ~x + 1;
    end
  endfunction

  reg [3:0] x;
  wire [3:0] y;

  assign y = foo(x);

  initial begin
    x = 4'b0110;
    #1;
    if (y==4'b1010)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
