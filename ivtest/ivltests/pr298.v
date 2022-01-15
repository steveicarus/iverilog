/*
 * Copyright (c) 2001 Philip Blundell
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
primitive p (Q, D);
input D;
output Q;
reg Q;
initial Q = 1'b0;
table
   0 : ? : 0;
   1 : ? : 1;
endtable
endprimitive

module m;

reg D;
wire Q;
reg A;
wire QQ;

p(Q, D);
buf(QQ, Q);

initial
  begin
     // The #1 is needed here to allow the initial values to
     // settle. Without it, there is a time-0 race.
    #1 $display(QQ, Q);
    #10
    D = 0;
    #15
    $display(QQ, Q);
    #20
    D = 1;
    #25
    $display(QQ, Q);
    $finish(0);
  end
endmodule
