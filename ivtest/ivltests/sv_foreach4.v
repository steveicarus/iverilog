/*
 * Copyright (c) 2014 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

`default_nettype none

class test_t;
   reg [1:0] a;
   reg [2:0] b;

   function new (int ax, int bx);
      begin
	 a = ax;
	 b = bx;
      end
   endfunction // new

endclass // test_t

module main;

   class container_t;
      test_t foo [0:3][0:7];

      function new();
	 bit [3:0] idx1, idx2;
	 begin
	    for (idx1 = 0 ; idx1 < 4 ; idx1 = idx1+1) begin
	       for (idx2 = 0 ; idx2 <= 7 ; idx2 = idx2+1)
		 foo[idx1][idx2] = new(idx1,idx2);
	    end
	 end
      endfunction // new

      task run();
	 bit [3:0] idx1, idx2;
	 test_t tmp;
	 foreach (foo[ia,ib]) begin
	    if (ia > 3 || ib > 7) begin
	       $display("FAILED -- index out of range: ia=%0d, ib=%0d", ia, ib);
	       $finish;
	    end

	    tmp = foo[ia][ib];
	    if (tmp.a !== ia[1:0] || tmp.b !== ib[2:0]) begin
	       $display("FAILED -- foo[%0d][%0d] == %b", ia, ib, {tmp.a, tmp.b});
	       $finish;
	    end

	    foo[ia][ib] = null;
	 end

	 for (idx1 = 0 ; idx1 < 4 ; idx1 = idx1+1) begin
	    for (idx2 = 0 ; idx2 <= 7 ; idx2 = idx2+1)
	      if (foo[idx1][idx2] != null) begin
		 $display("FAILED -- foreach failed to visit foo[%0d][%0d]", idx1,idx2);
		 $finish;
	      end
	 end
      endtask // run

   endclass

   container_t dut;
   initial begin
      dut = new;
      dut.run;
      $display("PASSED");
   end

endmodule // main
