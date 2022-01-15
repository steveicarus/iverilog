/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 * This test uses the level sensitive wait to notice the scheduling of
 * the expression evaluation. The FAILED message detects cases where
 * the value of test is true, even when the value of a is set to
 * false.
 */

module main;

   reg a;

   wire test = a == 1'b1;

   always #1 wait (test) begin
      if (a !== 1'b1) begin
	 $display("FAILED -- a == %b, test == %b", a, test);
	 $finish;
      end

      a = 1'b0;
   end

   initial begin
      a = 0;

      #10 a = 1;
      #10 a = 0;
      #10 $display("PASSED");
   end
endmodule // main
