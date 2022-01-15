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

// $Id: ldelay4.v,v 1.3 2007/12/06 02:31:10 stevewilliams Exp $

// Test for delays in structural logic.  Differential clock receiver UDP.

module test;

   wire q, e;
   reg  a, b;
   drec #1 rec(q, a, b);
   edet det (e, q);

   reg	error;
   initial
     begin
	error = 0;
	#2;
	forever @(e)
	  if (e !== 1'bx) begin // Fail on anything other then x.
	     error = 1;
	     $display("%0d: FAILED: e=%b", $time, e);
	  end
     end

   always @(q)
     $display("%d: q=%b", $time, q);

   initial
     begin
//	$dumpvars;
	a = 0;
	b = 1;
	#3;
	a = 1;
	b = 0;
	#2;
	a = 0;
	b = 1;
	#3;
	if (!error)
	  $display("PASSED");
     end
endmodule

// differential receiver

primitive drec (q, a, b);
   output        q;
   input  a, b;
   table
          1  0 : 1 ;
          0  1 : 0 ;
   endtable
endprimitive

// flag any edges to or from 'bx

primitive edet (q, i);
   output           q;
   input  i;
   reg	        q;
   table
         (?x) : ? : 1;
         (x?) : ? : 0;
   endtable
endprimitive
