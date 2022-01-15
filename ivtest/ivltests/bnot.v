module test;
   reg a,b;
   integer x;
   initial begin
      x=10;

      // ok
      a=x;

      // fails at run time with
      // vvm_func.cc:49: failed assertion `v.nbits == p.nbits'
      // Abort (core dumped)
      b = ~x;

      if (b === 1'b1)
	$display("PASSED");
      else
	$display("FAILED --- b = %b", b);

   end // initial begin
endmodule // test



/*
 * Copyright (c) 2000 Gerard A. Allan (gaa@ee.ed.ac.uk)
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
