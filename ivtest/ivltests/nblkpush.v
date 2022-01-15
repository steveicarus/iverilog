//
// Copyright (c) 2001 Stephan Boettcher <stephan@nevis.cloumbia.edu>
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
// Validates Non-blocking assignment propagation
// $Id: nblkpush.v,v 1.2 2005/07/07 16:25:20 stevewilliams Exp $

// Update: This test has a race in it that makes it not valid. The
// assumption that a blocking assign will push through the continuous
// assignment before the thread doing the assign is allowed to advance
// is not valid. This test only passes Verilog XL. Every other tool,
// commercial or otherwise, seems to FAIL this test. Therefore, this
// test should not be relied on.

module test;

   reg a, b, c, d;

   wire ab = a & b;
   wire abc = ab | c;
   wire abcd = abc & d;

   initial
     begin
	a = 0;
	b = 1;
	c = 0;
	d = 1;
	#1;
	a = 1;
	if (abcd === 1)
	  begin
	     $display("PASSED");
	     $finish;
	  end

	$display("FAILED ab=%b, abc=%b, abcd=%b", ab, abc, abcd);
	#1;
	if (abcd === 1)
	  $display("abcd value changed late");
	else
	  $display("abcd value still wrong");
     end

endmodule
