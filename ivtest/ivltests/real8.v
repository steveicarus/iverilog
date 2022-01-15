/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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
/* $Id: real8.v,v 1.1 2005/07/07 16:26:05 stevewilliams Exp $ */

/*
 * This test demonstrates (and checks) the handling of real values and
 * expressions in some ways that baseline IEEE1364-2001. Particularly,
 * reg and wire values with types, and nets able to carry real values
 * through delays.
 */
module main;

   // This is *not* valid baseline Verilog, but iverilog extensions
   // support this. These statements declare a real valued variable
   // and a real valued net.
   reg  real target;
   wire real feedback;

   // The feedback should take the target value 10 time units
   // after any change in the target value.
   assign #(10) feedback = target;

   // The control value is calculated from the current target
   // and feedback values. This is recalculated whenever either
   // of the inputs change.
   wire real control = (feedback - target)/2.0;

   initial begin
      target = 16.0;
      // The target should, after this assignment, have a well
      // defined value 16, but the feedback should remain at NaN
      // for a while.
      #1 $display($time,,"target=%f, feedback=%f, control=%f",
		  target, feedback, control);

      if (target != 16.0) begin
	 $display("FAILED -- target value is not correct.");
	 $finish;
      end

      // feedback is still undefined.

      // By now, the feedback as taken on a value, and the control
      // should have been calcluated.
      #10 $display($time,,"target=%f, feedback=%f, control=%f",
		  target, feedback, control);

      if (feedback != 16.0) begin
	 $display("FAILED -- feedback has wrong value.");
	 $finish;
      end

      if (control != 0.0) begin
	 $display("FAILED -- control has wrong value.");
	 $finish;
      end

      target = 8.0;

      #9 $display($time,,"target=%f, feedback=%f, control=%f",
		  target, feedback, control);

      if (feedback != 16.0) begin
	 $display("FAILED -- feedback has wrong value.");
	 $finish;
      end

      if (control != 4.0) begin
	 $display("FAILED -- control has wrong value.");
	 $finish;
      end

      #2 $display($time,,"target=%f, feedback=%f, control=%f",
		  target, feedback, control);

      if (feedback != 8.0) begin
	 $display("FAILED -- feedback has wrong value.");
	 $finish;
      end

      if (control != 0.0) begin
	 $display("FAILED -- control has wrong value.");
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
