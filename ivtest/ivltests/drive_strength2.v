/*
 * Copyright (c) 2000 Yasuhisa Kato <ykato@mac.com>
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

module  drvz( clk, iA, iC, ioS );
    input   clk, iA, iC ;
    inout   ioS ;

    assign  ioS = (iC) ? iA : 'bz ;

endmodule

module  main;

    reg     clk, c ;
    initial begin clk = 0 ; forever #5 clk = ~clk ; end
    initial begin c = 0 ; #40 $finish(0); end

    wire    a, b, s ;

    assign  a = 'b0 ;
    assign  b = 'b1 ;

    always @(posedge clk) c <= ~c ;

    drvz M ( clk, a, c,  s ) ;
    drvz N ( clk, b, ~c, s ) ; // line(A)

    always @(posedge clk)
        $display("%b %b %b", s, a, b );

endmodule

//  expected output
//  1 0 1
//  0 0 1
//  1 0 1
//  0 0 1

//  ivl 0.3 result
//  x 0 1
//  0 0 1
//  x 0 1
//  0 0 1
