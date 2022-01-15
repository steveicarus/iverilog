/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
 * This program demonstrates a tricky aspect of the Verilog syntax.
 * The problem is with the repeat statement. In fact, there is a
 * question as to whether it is a repeat statement at all, or an
 * event statement with a repeat modifier. These are the possibilities:
 *
 *    procedural_timing_control_statement ::=
 *          delay_or_event_control  statement_or_null
 *
 *    delay_or_event_control ::=
 *          event_control
 *          | repeat ( expression ) event_control
 *
 * If this interpretation is used, then ``repeat (5) @(posedge clk)''
 * should be taken as a delay_or_event_control and the thread will
 * block until the 5th clk posedge.
 *
 *    loop_statement ::=
 *         repeat ( expression ) statement
 *
 * If *this* interpretation is used, then ``repeat (5)'' is the loop
 * head is used and the statement in the example is executed 5 times.
 *
 * These two interpretations both appear to be perfectly valid. However,
 * real tools use the loop_statement, so the standard must be considered
 * broken and this interpretation used.
 */

module main;

   reg clk = 1;
   always #5 clk = ~clk;

   initial #1
      repeat (5) @(posedge clk) begin
	 if ($time !== 10) begin
	    $display("FAILED -- $time = %t", $time);
	    $finish;
	 end

	 $display("PASSED");
	 $finish;
      end

endmodule // main
