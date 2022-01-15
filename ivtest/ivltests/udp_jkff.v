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

// $Id: udp_jkff.v,v 1.2 2007/08/29 00:01:22 stevewilliams Exp $

module test_jkff;

   reg cp;
   reg j, k;
   reg s, r;
   reg qq;

   integer errors;
   initial errors = 0;

   initial
     begin
	cp <= 0;
	#10 {s,r, qq} <= 3'b 10_1;
	#10 {s,r, qq} <= 3'b 00_1;
	#10 {s,r, qq} <= 3'b x0_1;
	#10 {s,r, qq} <= 3'b 00_1;
	#10 {s,r, qq} <= 3'b 01_0;
	#10 {s,r, qq} <= 3'b 00_0;
	#10 {s,r, qq} <= 3'b x0_x;
	#10 {s,r, qq} <= 3'b 00_x;
	#10 {s,r, qq} <= 3'b 10_1;
	#10 {s,r, qq} <= 3'b 11_x;
	#10 {s,r, qq} <= 3'b 01_0;
	#10 {s,r, qq} <= 3'b 00_0;
	#10 {s,r, qq} <= 3'b 01_0;
	#10 {s,r, qq} <= 3'b 11_x;
	#10 {s,r, qq} <= 3'b 01_0;
	#10 {s,r, qq} <= 3'b 00_0;
	#10 {cp, j,k, qq} <= 4'b 0_00_0;
	#10 {cp, j,k, qq} <= 4'b 1_00_0;
	#10 {cp, j,k, qq} <= 4'b 0_01_0;
	#10 {cp, j,k, qq} <= 4'b 1_01_0;
	#10 {cp, j,k, qq} <= 4'b 0_11_0;
	#10 {cp, j,k, qq} <= 4'b 1_11_1;
	#10 {cp, j,k, qq} <= 4'b 0_11_1;
	#10 {cp, j,k, qq} <= 4'b 1_11_0;
	#10 {cp, j,k, qq} <= 4'b 0_00_0;
	#10 {cp, j,k, qq} <= 4'b x_00_0;
	#10 {cp, j,k, qq} <= 4'b 0_10_0;
	#10 {cp, j,k, qq} <= 4'b 1_10_1;
	#10 {cp, j,k, qq} <= 4'b x_10_1;
	#10 {cp, j,k, qq} <= 4'b 1_10_1;
	#10 {cp, j,k, qq} <= 4'b 0_10_1;
	#10 {cp, j,k, qq} <= 4'b x_10_1;
	#10 {cp, j,k, qq} <= 4'b 0_01_1;
	#10 {cp, j,k, qq} <= 4'b x_01_x;
	#10 {cp, j,k, qq} <= 4'b 1_01_x;
	#10 {cp, j,k, qq} <= 4'b 0_11_x;
	#10 {cp, j,k, qq} <= 4'b 1_11_x;
	#10 {cp, j,k, qq} <= 4'b 0_01_x;
	#10 {cp, j,k, qq} <= 4'b 1_01_0;
	#10 {cp, j,k, qq} <= 4'b x_11_0;
	#10 {cp, j,k, qq} <= 4'b 1_11_x;
	#10 {cp, j,k, qq} <= 4'b 0_10_x;
	#10 {cp, j,k, qq} <= 4'b 1_10_1;
	#10 {cp, j,k, qq} <= 4'b 0_00_1;
	#10;
	if (errors > 0)
	  $display("FAILED");
	else
	  $display("PASSED");
	#10 $finish;
     end

   wire q;
`ifdef FAKE_UDP
   // to get a vvp template, from which to build a UDP test vvp
   and ff (q, j, k, s, r);
`else
   jkff ff (q, cp, j, k, s, r);
`endif

   always @(cp or j or k or s or r)
     begin
	#2;
	$display("cp=%b j=%b k=%b s=%b r=%b  q=%b", cp, j, k, s, r, q);
	if (q !== qq && $time > 2)
	  begin
	     $display("FAILED: expect        q=%b  (time=%t)", qq, $time);
	     errors = errors + 1;
	  end
     end

endmodule

`ifdef FAKE_UDP
`else
primitive jkff(q, cp, j, k, s, r);
   output q;
   input  cp, j, k, s, r;
   reg	  q;
   table
   // (cp)  jk  s   r  :  q  :  q  ;
        ?   ?? (?0) 0  :  ?  :  -  ;
        ?   ??  0 (?0) :  ?  :  -  ;
        ?   *?  0   0  :  ?  :  -  ;
        ?   ?*  0   0  :  ?  :  -  ;
        ?   ??  1   0  :  ?  :  1  ;
        ?   ??  0   1  :  ?  :  0  ;
        ?   ??  x   0  :  1  :  1  ;
        ?   ??  0   x  :  0  :  0  ;
      (?0)  ??  0   0  :  ?  :  -  ;
      (1x)  ??  0   0  :  ?  :  -  ;
      (?1)  0?  0   0  :  0  :  0  ;
      (?1)  ?0  0   0  :  1  :  1  ;
      (0x)  0?  0   0  :  0  :  0  ;
      (0x)  ?0  0   0  :  1  :  1  ;
      (01)  1?  0   0  :  0  :  1  ;
      (01)  ?1  0   0  :  1  :  0  ;
      (01)  10  0   0  :  x  :  1  ;
      (01)  01  0   0  :  x  :  0  ;
   endtable
endprimitive
`endif
