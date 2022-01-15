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
// $Id: dotinid.v,v 1.1 2001/06/26 00:32:18 sib4 Exp $
// $Log: dotinid.v,v $
// Revision 1.1  2001/06/26 00:32:18  sib4
// Two new tests for identifier parsing/elaboration
//
//
// IVL parser/elaboration test for escaped names with .

module a;
   wire \a.b ;
   m    \c.d (\a.b );
   initial
     begin
	\c.d . \y.z  <= 1'b1;
	#1;
	if (\a.b  === 1'b1)
	  $display("PASSED");
	else
	  $display("FAILED");
     end
endmodule
module m(x);
   output x;
   reg	  \y.z ;
   assign x = \y.z ;
endmodule
