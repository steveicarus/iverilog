/*
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

// $Id: deposit.v,v 1.4 2001/11/22 04:36:33 sib4 Exp $

// Test for vpi_put_value() to properly propagate in structural context.

module deposit_test;

   reg ck;

   reg start;
   initial start = 0;

`ifdef RTL

   reg [3:0] cnt;
   wire      cnt_tc = &cnt;

   always @(posedge ck)
     if (start | ~cnt_tc)
       cnt <= cnt + 1;

`else // !ifdef RTL

   wire [3:0] cnt;
   wire [3:0] cnt_1;
   wire [3:0] cnt_c;
   wire       cnt_tc;
   wire       ne, e;

   and (cnt_tc, cnt[0], cnt[1], cnt[2], cnt[3]);
   not (ne, cnt_tc);
   or (e, ne, start);

   had A0 (cnt[0],     1'b1,  cnt_c[0], cnt_1[0]);
   had A1 (cnt[1], cnt_c[0],  cnt_c[1], cnt_1[1]);
   had A2 (cnt[2], cnt_c[1],  cnt_c[2], cnt_1[2]);
   had A3 (cnt[3], cnt_c[2],  cnt_c[3], cnt_1[3]);

   dffe C0 (ck, e, cnt_1[0], cnt[0]);
   dffe C1 (ck, e, cnt_1[1], cnt[1]);
   dffe C2 (ck, e, cnt_1[2], cnt[2]);
   dffe C3 (ck, e, cnt_1[3], cnt[3]);

`endif // !ifdef RTL

   integer    r0; initial r0 = 0;
   integer    r1; initial r1 = 0;

   always
     begin
	#5 ck <= 0;
	#4;
	$display("%b %b %d %d", cnt, cnt_tc, r0, r1);
	if (cnt_tc === 1'b0) r0 = r0 + 1;
	if (cnt_tc === 1'b1) r1 = r1 + 1;
	#1 ck <= 1;
     end

   initial
     begin
	// $dumpfile("deposit.vcd");
	// $dumpvars(0, deposit_test);
	#22;
`ifdef RTL
	cnt <= 4'b 1010;
`else
	$deposit(C0.Q, 1'b0);
	$deposit(C1.Q, 1'b1);
	$deposit(C2.Q, 1'b0);
	$deposit(C3.Q, 1'b1);
`endif
	#1 if (cnt !== 4'b1010)
	  $display("FAILED");
	#99;
	$display("%d/%d", r0, r1);
	if (r0===5 && r1===5)
	  $display("PASSED");
	else
	  $display("FAILED");
	$finish;
     end

endmodule

`ifdef RTL
`else

module dffe (CK, E, D,  Q);
   input  CK, E, D;
   output Q;
   wire   qq;
   UDP_dffe ff (qq, CK, E, D);
   buf #1 (Q, qq);
endmodule

primitive UDP_dffe (q,  cp, e, d);
   output                q;
   reg	            q;
   input cp, e, d;
   table
        (01) 1  1 : ? :  1 ;
        (01) 1  0 : ? :  0 ;
         *   0  ? : ? :  - ;
         *   ?  1 : 1 :  - ;
         *   ?  0 : 0 :  - ;
        (1x) ?  ? : ? :  - ;
        (?0) ?  ? : ? :  - ;
         ?   ?  * : ? :  - ;
         ?   *  ? : ? :  - ;
    endtable
endprimitive

module had (A, B,  C, S);
   input A, B;
   output C, S;
   xor s (S, A, B);
   and c (C, A, B);
endmodule

`endif // !ifdef RTL
