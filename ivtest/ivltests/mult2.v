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
 * Test some multiply values for an 18*18-->36 multiply.
 */

module main;

   wire [35:0] p;
   reg [17:0]  a, b;
   reg	       clk, ce, reset;

   parameter   MAX_TRIALS = 1000;
   integer     idx;

   MULT18X18S dut (p, a, b, clk, ce, reset);

   initial begin
      clk   <= 0;
      ce    <= 1;
      reset <= 1;
      a     <= 0;
      b     <= 0;
      #5 clk <= 1;
      #5 clk <= 0;

      if (p !== 36'h0) begin
	 $display("FAILED -- reset p=%h", p);
	 $finish;
      end

      reset <= 0;

      /* A magical value I know failed at one time. */
      a <= 18'h3ff82;
      b <= 18'h04000;

      #5 clk <= 1;
      #5 clk <= 0;

      if (p !== 36'hfffe08000) begin
	 $display("FAILED -- %h * %h --> %h", a, b, p);
	 $finish;
      end

      for (idx = 0 ;  idx < MAX_TRIALS ;  idx = idx + 1) begin
	 a <= $random;
	 b <= $random;

	 #5 clk <= 1;
	 #5 clk <= 0;

	 if ($signed(p) !== ($signed(a) * $signed(b))) begin
	    $display("FAILED == %h * %h --> %h", a, b, p);
	    $finish;
	 end
      end // for (idx = 0 ;  idx < `MAX_TRIALS ;  idx = idx + 1)

      $display("PASSED");
   end // initial begin

endmodule // main

module MULT18X18S (output reg [35:0] P,
		   input  [17:0] A,
		   input  [17:0] B,
		   input C, CE, R);

   wire [35:0] a_in = { {18{A[17]}}, A[17:0] };
   wire [35:0] b_in = { {18{B[17]}}, B[17:0] };
   wire [35:0] p_in;
   reg [35:0]  p_out;

   assign p_in = a_in * b_in;

   always @(posedge C)
     if (R)
       P <= 36'b0;
     else if (CE)
       P <= p_in;


endmodule
