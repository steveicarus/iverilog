/*
 * Copyright (c) 2006 Stephen Williams (steve@icarus.com)
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
 *
 *   $Id: casesynth1.v,v 1.1 2006/01/01 01:01:31 stevewilliams Exp $"
 */


module main;

   reg clk, rst, set;
   reg [3:0] out, load;
   reg [1:0] op;

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge rst)
     if (rst) begin
	out <= 0;

     end else if (set) begin
	out <= load;

     end else
       case (op)
	 2'b01: /* increment */ out <= out + 1;
	 2'b10: /* decrement */ out <= out - 1;
	 2'b11: /* Invert    */ out <= ~out;
	 /* Other ops cause out to not change. */
       endcase // case(mod)


   (* ivl_synthesis_off *)
   initial begin
      /* Test rst behavior. */
      op = 2'b00;
      rst = 1;
      set = 0;
      load = 0;
      clk = 0;

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b0000) begin
	 $display("FAILED -- out=%b (reset)", out);
	 $finish;
      end

      /* Test set behavior */
      rst = 0;
      set = 1;
      load = 4'b0100;
      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b0100) begin
	 $display("FAILED -- out=%b (load)", out);
	 $finish;
      end

      /* Test increment behavior */
      op = 2'b01;
      rst = 0;
      set = 0;
      load = 0;
      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b0101) begin
	 $display("FAILED -- out=%b (increment 1)", out);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b0110) begin
	 $display("FAILED -- out=%b (increment 2)", out);
	 $finish;
      end

      /* Test invert behavior */
      op = 2'b11;

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b1001) begin
	 $display("FAILED == out=%b (invert)", out);
	 $finish;
      end

      /* Test NO-OP behavior */
      op = 2'b00;

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b1001) begin
	 $display("FAILED -- out=%b (noop)", out);
	 $finish;
      end

      /* Test decrement behavior */
      op = 2'b10;

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b1000) begin
	 $display("FAILED -- out=%b (decrement 1)", out);
	 $finish;
      end

      #1 clk = 1;
      #1 clk = 0;

      if (out !== 4'b0111) begin
	 $display("FAILED -- out=%b (decrement 2)", out);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule // main
