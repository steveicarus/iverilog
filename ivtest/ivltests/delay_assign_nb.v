//
// Copyright (c) 2001 Stephen Williams <steve@icarus.com>
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

/*
 * This program tests the behavior of a simple non-blocking assignment
 * with an internal delay. We can check that the value changes at the
 * right time and not the wrong time.
 */

module main ;

   reg a;

   initial begin
      a = 0;
      if (a !== 0) begin
	 $display("FAILED -- a at 0 is %b", a);
	 $finish;
      end

      a <= #2 1;

      if (a !== 0) begin
	 $display("FAILED -- (0) a should still be 0 but is %b", a);
	 $finish;
      end

      #1 if (a !== 0) begin
	 $display("FAILED -- (1) a should still be 0 but is %b", a);
	 $finish;
      end

      #2 if (a !== 1'b1) begin
	 $display("FAILED -- a should now be 1, but is %b", a);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
