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
  wire [3:0] a,b,c,d,e,f;

  ma a1(a);
  ma #(4'd9) a2(b);
  mb #(4'd12) b1(c);
  mb b2(d);
  mc #(4'd2) c1(e,f);

  initial begin
    #1;
    if (a===4'd5 && b===4'd9 && c===4'd12 && d===4'd7 && e===4'd2 && f===4'd3)
      $display("PASSED");
    else
      $display("FAILED");
  end
endmodule

module ma(x);
  output [3:0] x;
  parameter P = 4'd5;
  assign x = P;
endmodule

module mb(x);
  output [3:0] x;
  parameter Q = 4'd7;
  ma #(Q) a1(x);
endmodule

module mc(x,y);
  output [3:0] x,y;
  parameter R = 4'd1;
  parameter S = 4'd3;
  ma #(R) a1(x);
  ma #(S) a2(y);
endmodule
