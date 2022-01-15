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
  reg [3:0] a,b;
  wire c;

  assign c = a && b;

  initial begin
    a = 4'd3;
    b = 4'd6;
    #1;
    if (c!==1) begin
      $display("FAILED");
      $finish;
    end
    a = 4'd0;
    #1;
    if (c!==0) begin
      $display("FAILED");
      $finish;
    end
    b = 4'd0;
    #1;
    if (c!==0) begin
      $display("FAILED");
      $finish;
    end
    $display("PASSED");
  end
endmodule
