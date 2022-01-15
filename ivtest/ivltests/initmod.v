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

module  busm ( clk, iB, oB );
    input           clk ;
    input   [3:0]   iB ;
    output  [3:0]   oB ;
    reg     [3:0]   r ;

    assign  oB = r ;
    always @(posedge clk) r <= iB ;

endmodule

module  main;
    reg     a, b, c, d ;

    reg     clk ;
    initial begin clk = 0 ; forever #5 clk = ~clk ; end
    initial begin a = 0 ; c = 0 ; #100 $finish(0); end

    wire    e0, f0, g0, h0 ;
    wire    e, f, g, h ;
    wire    [3:0]   ii, oo ;

    always @(posedge clk) a <= ~a ;
    always @(posedge clk) b <=  a ;
    always @(posedge clk) c <=  c ^ a ;
    always @(posedge clk) d <= ~c ;

    assign  ii = {a, b, c, d} ;
    assign  {e0, f0, g0, h0} = oo ;

    busm M0 ( clk, ii, oo );
    busm M1 ( clk, {a,b,c,d}, {e,f,g,h} );

    always @(posedge clk)
        $display("%h %h %h %h : %b : %h %h %h %h : %b : %h %h %h %h",
            a, b, c, d, M0.r, e0, f0, g0, h0,
            M1.r, e, f, g, h

        );

endmodule

//  expecting result
//  0 x 0 x : xxxx : z z z z : xxxx : z z z z
//  1 0 0 1 : 0z0z : 0 z 0 z : 0z0z : 0 z 0 z
//  0 1 1 1 : 1001 : 1 0 0 1 : 1001 : 1 0 0 1
//  1 0 1 0 : 0111 : 0 1 1 1 : 0111 : 0 1 1 1
//  0 1 0 0 : 1010 : 1 0 1 0 : 1010 : 1 0 1 0
//  1 0 0 1 : 0100 : 0 1 0 0 : 0100 : 0 1 0 0
//  0 1 1 1 : 1001 : 1 0 0 1 : 1001 : 1 0 0 1
//  1 0 1 0 : 0111 : 0 1 1 1 : 0111 : 0 1 1 1
//  0 1 0 0 : 1010 : 1 0 1 0 : 1010 : 1 0 1 0
//  1 0 0 1 : 0100 : 0 1 0 0 : 0100 : 0 1 0 0
