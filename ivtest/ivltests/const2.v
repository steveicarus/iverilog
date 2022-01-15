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
 * This test checks that signed comparisons of constants work
 * properly in conditional expressions. These cases are of
 * interest because they are evaluated at compile time, so
 * that dead code can be skipped.
 */
module main;

   initial begin
      if ((0 < -255) || (0 > 255)) begin
	 $display("FAILED -- expression evaluated true");
	 $finish;
      end

      if ((0 <= -255) || (0 >= 255)) begin
	 $display("FAILED -- expression evaluated true");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
