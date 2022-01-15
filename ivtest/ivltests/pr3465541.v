/***********************************************************************
 *
 * Copyright (C) 2011 Adrian Wise
 *
 * This source code is free software; you can redistribute it
 * and/or modify it in source code form under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 ***********************************************************************
 *
 * This is a testbench exercising gate-level modelling of DTL gates,
 * distilled down (as a test-case) from a much larger design.
 * The gates can only pull down strongly to ground and have a weak
 * pull-up.
 *
 * This illustrates a problem in the git master branch as of 24 December
 * 2011, where a gate that does not pull-up strongly cannot drive a bit
 * of a bus (to either logic level), but can drive a single-bit wire
 * correctly.
 *
 * This is an extended version of the test case provided with the
 * bug report, to cover part selects with a non-zero base, and to
 * make the error checking a bit more robust.
 **********************************************************************/

`timescale 1 ns / 100 ps

module dtl_inv (op, in1);
   output op;
   input  in1;
   not (strong0, pull1) #16 not1 (op, in1);
endmodule // dtl_inv

module top;

   reg        d;

   wire       w;
   wire [1:0] b;

   reg        pass;

   dtl_inv u_1 (.op(w),    .in1(d));
   dtl_inv u_2 (.op(b[0]), .in1(d));
   dtl_inv u_3 (.op(b[1]), .in1(b[0]));


   initial begin

      pass = 1'b1;

      d  = 1'b0;
      # 100;

      if ((w !== 1'b1) || (b[0] !== 1'b1) || (b[1] !== 1'b0)) begin
         $display("Failed (w !== b[0]): d = %b,  w = %b, b = %b", d, w, b);
         pass = 1'b0;
      end

      d  = 1'b1;
      # 100;
      if ((w !== 1'b0) || (b[0] !== 1'b0) || (b[1] !== 1'b1)) begin
         $display("Failed (w !== b[0]): d = %b,  w = %b, b = %b", d, w, b);
         pass = 1'b0;
      end

      if (pass) $display("PASSED");
   end

endmodule // top
