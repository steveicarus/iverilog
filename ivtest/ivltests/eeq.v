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
// $Id: eeq.v,v 1.1 2001/06/26 01:07:15 sib4 Exp $
// $Log: eeq.v,v $
// Revision 1.1  2001/06/26 01:07:15  sib4
// new test for === and !==
//
//
// Test for === amd !== in structural context.

module eeq;

   reg [3:0] a, b;

   wire      eeq = a === b;
`ifdef DONT_TEST_NEE
   wire      nee = ~(a === b);
`else
   wire      nee = a !== b;
`endif

   reg	     err;

   always
     begin
	#2;
	$display("%b %b ===%b !==%b", a, b, eeq, nee);
	if (((a === b) !== eeq) || ((a !== b) !== nee)) err = 1;
     end

   initial
     begin
	err = 0;
	#1 a = 4'b zx10; b = 4'b zx10; #1;
	#1 a = 4'b 1x10; b = 4'b zx10; #1;
	#1 a = 4'b xz10; b = 4'b zx10; #1;
	#1 a = 4'b xz01; b = 4'b zx10; #1;
	#1 a = 4'b 0000; b = 4'b 0000; #1;
	#1 a = 4'b 1111; b = 4'b 1111; #1;
	#1 a = 4'b xxxx; b = 4'b xxxx; #1;
	#1 a = 4'b zzzz; b = 4'b zzzz; #1;
	#1;
	if (err)
	  $display("FAILED");
	else
	  $display("PASSED");
	$finish;
     end

endmodule
