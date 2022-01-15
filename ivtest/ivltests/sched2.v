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

module main;

   reg a;
   reg b;
   wire q = a & b;

   initial begin
      a = 1;
      b = 0;
      #1;
      if (q !== 0) begin
	 $display("FAILED -- q did not start out right: %b", q);
	 $finish;
      end

      b = 1;
      if (q !== 0) begin
	 // Since b takes the new value with a blocking assignment,
	 // it is up to the & gate to schedule the q change, and not
	 // actually push the change through.
	 $display("FAILED -- q changed too soon? %b", q);
	 $finish;
      end

      if (b !== 1) begin
	 $display("FAILED -- b value did not stick: %b", b);
	 $finish;
      end

      // The #0 delay lets the scheduler execute the change to the
      // q value, so that we can read the correct value out.
      #0 if (q !== 1) begin
	 $display("FAILED -- q did not change when it should: %b", q);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
