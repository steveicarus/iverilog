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
// $Id: mangle_1.v,v 1.1 2001/06/19 13:52:13 ka6s Exp $
// $Log: mangle_1.v,v $
// Revision 1.1  2001/06/19 13:52:13  ka6s
// Added 4 tests from Stephan Boettcher
//
// Test of \escaped identifiers
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
   reg    \y.z ;
   assign x = \y.z ;
endmodule
