/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 * This is a check of the implementation of division and multiplication
 * within more complex expressions.
 */
module test;
   task mod;
      input  [31:0] a;
      input  [15:0] b;
      output [31:0] out;
      begin
	 out = a-(a/b)*b;
      end
   endtask

   reg [31:0] result,c, nl;
   initial begin
      c = 13; nl = 3;
      mod(c, nl, result);
      $display("13 %% 3 = %d", result);
      if (result !== 32'h00_00_00_01) begin
	 $display("FAILED -- result is %b", result);
	 $finish;
      end

      $display("PASSED");

  end
endmodule
