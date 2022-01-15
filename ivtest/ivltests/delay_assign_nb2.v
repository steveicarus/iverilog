//
// Copyright (c) 2002 Stephen Williams (steve@icarus.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//

/*
 * This function captures the correctness of a non-constant delay
 * that is internal to a non-blocking assignment.
 */

module main;

   reg [7:0] delay = 0;
   reg	     step;

   initial begin
      delay = 2;
      step = 0;
      step <= #(delay) 1;

      #1 if (step !== 0) begin
	 $display("FAILED -- step=%b at time=1", step);
	 $finish;
      end

      #2 if (step !== 1) begin
	 $display("FAILED == step=%b at time=3", step);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
