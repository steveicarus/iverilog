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
 * This test was inspired by PR#539. We check that the calculated bit
 * select of a net in a continuous assignment gets bits in the right
 * order and position. The trick here is that the bits are numbered
 * from MSB to LSB.
 */

module main;

   reg [0:63] target0 = 64'h0040200000000000;
   reg [1:64] target1 = 64'h0040200000000000;
   reg [6:0]  idx;

   wire       mux0 = target0[idx];
   wire       mux1 = target1[idx+1];

   initial begin
      $display( "Using constant indices:" );
      $display( "    %b=v[ 9]", target0[ 9] );
      if (target0[9] !== 1'b1) begin
	 $display("FAILED -- target0[9] != 1");
	 $finish;
      end

      $display( "    %b=v[18]", target0[18] );
      if (target0[18] !== 1'b1) begin
	 $display("FAILED -- target0[18] != 1");
	 $finish;
      end

      $display( "    %b=v[45]", target0[45] );
      if (target0[45] !== 1'b0) begin
	 $display("FAILED -- target0[45] != 0");
	 $finish;
      end

      $display( "    %b=v[54]", target0[54] );
      if (target0[54] !== 1'b0) begin
	 $display("FAILED -- target0[54] != 0");
	 $finish;
      end

      $display( "Using calcuated indices:" );
      for (idx = 0 ;  idx < 64 ;  idx = idx + 1) begin
	 #1 $display("target0[%2d]=%b, mux0=%b", idx,   target0[idx],   mux0);
	    $display("target1[%2d]=%b, mux1=%b", idx+1, target1[idx+1], mux1);

	 if (target0[idx] !== mux0) begin
	    $display("FAILED -- target0[idx] != mux0");
	    $finish;
	 end

	 if (target1[idx+1] !== mux1) begin
	    $display("FAILED -- target1[idx+1] != mux1");
	    $finish;
	 end
      end

      $display("PASSED");
   end

endmodule // main
