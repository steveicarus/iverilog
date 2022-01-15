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
//  SDW - assign a string in a function
//
//  This test was contributed via forwarding on the geda netlist. I don't
//  know who actually wrote it - that isn't obvious from the copy of email
//  I eventually received - SDW
//

module test();
wire [31:0] A;
reg  [31:0] B;

function [31:0] message;
        input [1:0] reg_num;
  begin
        message = (reg_num == 2'b00) ? "Mes0":
                  (reg_num == 2'b01) ? "Mes1":
                  (reg_num == 2'b10) ? "Mes2":
                                       "Mes3";
  end
endfunction

assign A = "hi";
initial
  begin
    B = "ho";
    #1;
    $display ("%s", A);
    $display ("%s", message(1));
    $finish(0);
  end
endmodule
