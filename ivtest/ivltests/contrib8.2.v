/*
 * Copyright (c) 1998 Philips Semiconductors (Stefan.Thiede@sv.sc.philips.com)
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
// 9/7/99 - SDW - Added a PASSED message - no functional checking needed


module test();
wire [1:0] a;
wire [9:0] b;
wire [0:9] d;
a       a1(.a(c));
b       b1(.a(a[0]));
c       ci(.a({a, b}));
d       d1(.a({d[0:9], a[1:0]}), .d(c));
f       f(a);
a       a3(a[1]);
a       a4({a[1]});
g       g({a,b});
e       e();

initial
  $display("PASSED");
endmodule

module a(a);
input a;
endmodule


module b(.a(b));
input b;
endmodule

module c(.a({b, c}), );
input [10:0] b;
input c;
endmodule

module d(.a({b, c}), d);
input [10:0] b;
input c, d;
endmodule

module e();
endmodule

module f({a, b});
input a, b;
endmodule
module g(a);
input [11:0] a;
endmodule
