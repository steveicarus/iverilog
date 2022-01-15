/*
 This incorrect code causes iverilog 20070421 and earlier to dump core.

  $ iverilog -t null empty_param.v
  Segmentation Fault - core dumped
 */

module param_test (clk, reset_n, test_expr);
    parameter severity_level = 1;
    parameter width = 32;
    parameter property_type  = 0;

    input     clk, reset_n;
    input [width-1:0] test_expr;

endmodule

module empty_param;
    reg clk;
    reg [3:0] fsm;

   // An empty parameter like is easy to cause with an undefined macro
   // expanding to null

    param_test #( , 4) submod(clk, 1'b1, fsm);

   initial begin
      if (submod.severity_level !== 1) begin
	 $display("FAILED -- severity_level = %d", submod.severity_level);
	 $finish;
      end

      if (submod.width !== 4) begin
	 $display("FAILED -- width = %d", submod.width);
	 $finish;
      end

      if (submod.property_type !== 0) begin
	 $display("FAILED -- property_type = %d", submod.property_type);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule



/* Copyright (C) 1999 Stephen G. Tell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * n
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 *
 */
