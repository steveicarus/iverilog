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
 * This program checks that the initial value of a memory is x. The
 * verilog standard clearly states that reg values must start as x
 * values, and implies that memories are the same.
 */
module  main;

    integer   mem [0:1] ;

   initial begin
      if (mem[0] !== 32'hxxxx) begin
	 $display("FAILED -- mem[0] == %b", mem[0]);
	 $finish;
      end

      if (mem[1] !== 32'hxxxx) begin
	 $display("FAILED -- mem[1] == %b", mem[1]);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
