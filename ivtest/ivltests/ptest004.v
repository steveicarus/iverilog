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
  wire [3:0] b;
  reg [1:0] a;
  reg c;

  assign b = c<<a;

  initial begin
    #1 ;
    c = 1'b1;
    a = 2'd2;
    #1;
    if (b===4'b0100)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule
