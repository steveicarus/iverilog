/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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

/*
 * Test that the repeat expression of a concatenation can have
 * a parameter expression in it.
 */
module test;
   parameter addrbits=10;
   parameter HiAdrsBitVal = { 1'b1, { (addrbits-1) { 1'b0 } } };
   initial begin
      if ($sizeof(HiAdrsBitVal) != 10) begin
	 $display("FAILED -- $sizeof(HiAdrsBitVal) is %d",
		  $sizeof(HiAdrsBitVal));
	 $finish;
      end
      $display("HiAdrsBitVal=%b, size=%d",
	       HiAdrsBitVal,
	       $sizeof(HiAdrsBitVal));
      $display("PASSED");
   end // initial begin


endmodule // test
