/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
 * This program checks that some basics of real value support work.
 */
module main;

   realtime x;
   real a3, a4;

   initial begin
      a3 = 0.3;
      a4 = 0.4;

      x = 2 * a4 + a3;

      $display("a3 = %f, a4 = %f, x = %f", a3, a4, x);
      if (x > 1.1001) begin
	 $display("FAILED");
	 $finish;
      end
      if (x < 1.0999) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end // initial begin
endmodule // main
