`begin_keywords "1364-2005"
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
 *
 *   $Id: sqrt32synth.v,v 1.2 2007/08/30 01:25:29 stevewilliams Exp $"
 */


 /*
  * This module approximates the square root of an unsigned 32bit
  * number. The algorithm works by doing a bit-wise binary search.
  * Starting from the most significant bit, the accumulated value
  * tries to put a 1 in the bit position. If that makes the square
  * too big for the input, the bit is left zero, otherwise it is set
  * in the result. This continues for each bit, decreasing in
  * significance, until all the bits are calculated or all the
  * remaining bits are zero.
  *
  * Since the result is an integer, this function really calculates
  * value of the expression:
  *
  *      x = floor(sqrt(y))
  *
  * where sqrt(y) is the exact square root of y and floor(N) is the
  * largest integer <= N.
  *
  * For 32bit numbers, this will never run more then 16 iterations,
  * which amounts to 16 clocks.
  */

module sqrt
  (input clk,
   output wire rdy,
   input reset,
   input [31:0] x,
   output reg [15:0] acc);


   //32
   parameter ss=5;  localparam w=1<<ss; //need to change in 2 places   1/2

   // acc holds the accumulated result, and acc2 is the accumulated
   // square of the accumulated result.
   //reg [w/2-1:0] acc;
   reg [w-1:0] acc2;

   // Keep track of which bit I'm working on.
   reg [ss-1:0]  bitl;
   wire [w/2-1:0] bit = 1 << bitl;
   wire [w-1:0] bit2 = 1 << (bitl << 1);

   // The output is ready when the bitl counter underflows.
   assign rdy = bitl[ss-1];

   // guess holds the potential next values for acc, and guess2 holds
   // the square of that guess. The guess2 calculation is a little bit
   // subtle. The idea is that:
   //
   //      guess2 = (acc + bit) * (acc + bit)
   //             = (acc * acc) + 2*acc*bit + bit*bit
   //             = acc2 + 2*acc*bit + bit2
   //             = acc2 + 2 * (acc<<bitl) + bit
   //
   // This works out using shifts because bit and bit2 are known to
   // have only a single bit in them.
   wire [w/2-1:0] guess  = acc | bit;
   wire [w-1:0] guess2 = acc2 + bit2 + ((acc << bitl) << 1);

   (* ivl_synthesis_on *)
   always @(posedge clk or posedge reset)
      if (reset) begin
         acc = 0;
         acc2 = 0;
         bitl = w/2-1;
      end else begin
         if (guess2 <= x) begin
            acc  <= guess;
            acc2 <= guess2;
         end
         bitl <= bitl - 1;
      end

endmodule // sqrt



module testBench;

   parameter ss=5;  parameter w=1<<ss; //need to change in 2 places   2/2
   //parameter Amax= 2000000;
   parameter  Amax= 10001; //quick test

   reg [w-1:0] A;
   reg	       clk, reset;

   wire [(w/2)-1:0] Z;
   wire		    done;

   sqrt dut (.clk(clk), .rdy(done), .reset(reset), .x(A), .acc(Z));

   (* ivl_synthesis_off *)
   always #5 clk = !clk;

   task reset_dut ;
      begin
	 reset = 1;
	 @(posedge clk);
	 #1 reset = 0;
	 @(negedge clk);
      end
   endtask

   task run_dut ;
      begin
	 while (done==0)
	   begin
	      @(posedge clk);
	   end
      end
   endtask

   integer idx, a, z, errCnt;

   (* ivl_synthesis_off *)
   initial
     begin
	reset = 0;
	clk = 0;

	$display ("ss=%d, width=%d, Amax=%d", ss, w, Amax);
	errCnt = 0;

	A = 4;
	reset_dut;
	run_dut;
	$display("test=0 x=%d, y=%d", A, Z);

	A = Amax/10;
	reset_dut;
	run_dut;
	$display("test=0 x=%d, y=%d", A, Z);

	A = Amax-1;
	reset_dut;
	run_dut;
	$display("test=0 x=%d, y=%d", A, Z);

	for (idx = 1 ;  idx < Amax;  idx = 2*idx) begin
           A = idx;
           reset_dut;
           run_dut;

           $display("%d: x=%d, y=%d", idx, A, Z);

	   a = A;
	   z = Z;

           if (a < (z *z)) begin
	      $display("test=%d x=%d, y=%d ERROR:y is too big", idx, A, Z);
	      $display("FAILED");
	      $finish;
           end

	   if (z<65535)  // at this number y*y overflows, so cannot test this way
	     begin
		if (a >= ((z + 1)*(z + 1)))
		  begin
		     $display("test=%d x=%d, y=%d ERROR: y is too small", idx, A, Z);
		     $display("FAILED");
		     $finish;
		  end
	     end
	   else
	     begin
		$display ("Could not verify above number");
	     end

	end

	$display ("Running  tests Amax=%d random input numbers", Amax);

	for (idx = 0 ;  idx < Amax;  idx = 1+ idx) begin
           A = $random;
           // A = A - ((A / Amax) * Amax);	 //this is needed only if <32 bit

           //A = idx;    //sequential -- comment out to get random tests

	   if (A < 1<<(w-1)) begin

              reset_dut;
              run_dut;

              //$display("%d: x=%d, y=%d", idx, A, Z);

	      a = A;
	      z = Z;

	      if (a < (z *z)) begin
		 $display("test=%d x=%d, y=%d ERROR:y is too big", idx, A, Z);
		 $display("FAILED");
		 $finish;
	      end

	      if (z<65535)  // at this number y*y overflows, so cannot test this way
		begin
		   if (a >= ((z + 1)*(z + 1)))
		     begin
			$display("test=%d x=%d, y=%d ERROR: y is too small", idx, A, Z);
			$display("FAILED");
			$finish;
		     end
		end
	      else
		begin
		   $display ("Could not verify above number");
		end


              //$display("%d: x=%d, y=%d", idx, A, Z);
	      if (idx%1000 == 0) $display("Finished %d tests", idx);
	   end // if (A < Amax) begin
	end
	$display ("PASSED");
	$finish;

     end

endmodule
`end_keywords
