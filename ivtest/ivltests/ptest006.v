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
  reg c;
  reg [3:0] a,b;
  wire [3:0] d;

  assign d = c ? a : b;

  initial begin
    a = 4'd5;
    b = 4'd6;
    c = 0;
    #1;
    if (d!==4'd6) begin
      $display("FAILED");
      $finish;
    end
    c = 1;
    #1;
    if (d!==4'd5) begin
      $display("FAILED");
      $finish;
    end
    $display("PASSED");
  end
endmodule
