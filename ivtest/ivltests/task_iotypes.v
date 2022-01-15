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
 * This program, based on PR#150, need only compile. It is testing
 * the syntax of giving types to task ports. I'm careful to *not*
 * invoke this task because there are potential optimization gotchas
 * that have been known to trip up the compiler. Specifically, ports
 * that are unused in a task that is not called can cause crashes in
 * some Icarus Verilog versions
 */

module gen_errors;
   task A;
     input  B;
     integer B;
     output  C;
     integer C;
     output  D;
     reg     D;
     inout [31:0] E;
     reg   [31:0] E;
     input [15:0] F;
     reg   [15:0] F;
     begin
       C = B;
     end
   endtask

   initial begin
      $display("PASSED");
   end

endmodule
