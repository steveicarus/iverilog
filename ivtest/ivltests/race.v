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
 * This sample demonstrates the post 20030904 Icarus Verilog feature
 * where combinational blocks with time-0 races against the rest of
 * the design can be resolved.
 *
 * The always @(foo) threads should be detected by the compiler as
 * combinational, and should be pushed to the front of the time-0
 * scheduling queue. This causes the threads to enter the wait early
 * so that it can detect the change from x to 1 for its value.
 *
 * The program HAS a time-0 race according to the IEEE1364 standard,
 * but Icarus Verilog as an extension resolves this race intentionally
 * as described.
 */

module main;

   reg foo, bar;
   reg foo_ok = 0, bar_ok = 0;

   initial foo = 1;

   always @(foo) begin
      if (foo !== 1'b1) begin
	 $display("FAILED --(foo = %b)", foo);
	 $finish;
      end
      foo_ok = 1;
   end

   always @(bar) begin
      if (bar !== 1'b1) begin
	 $display("FAILED --(bar = %b)", bar);
	 $finish;
      end
      bar_ok = 1;
   end

   initial bar = 1;

   initial begin
      #1 if (foo_ok !== 1) begin
	 $display("FAILED -- foo lost the race");
	 $finish;
      end

      if (bar_ok !== 1) begin
	 $display("FAILED -- bar lost the race");
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
