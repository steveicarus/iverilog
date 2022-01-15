/*
 * Copyright (c) 2002 Jane Skinner
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

// Icarus 0.6, snapshot 20020728 or snapshot 20010806
// ==================================================
// -- confused by complex disables
//
// -- to run, incant
//                  iverilog tt.v
//                  vvp a.out
//
// Veriwell
// ========
// -- OK
//
module top;

  integer loop_cntr, simple_fail, loop_fail;
  reg fred, abort;

  initial begin
    #1;
    simple_fail = 0;
    loop_fail = 0;
    fred = 0;
    abort = 1;
    #4;
    fred = 1;
    #4
    if(simple_fail) $display("\n***** simple block disable FAILED *****");
    else            $display("\n***** simple block disable PASSED *****");
    if(loop_fail) $display("***** complex block & loop disable FAILED *****\n");
    else          $display("***** complex block & loop disable PASSED *****\n");
    $finish(0);
  end

  // simple block disable
  initial begin: block_name
    #2;
    disable block_name;
    simple_fail = 1;
  end

  // more complex: block disable inside for-loop
  initial begin
    #2;
    begin: configloop
      for (loop_cntr = 0; loop_cntr < 3; loop_cntr=loop_cntr+1) begin
	wait (fred);
	if (abort) begin
	  disable configloop;
	end
	loop_fail = 1;
      end
    end // configloop block
    if (loop_fail) $display("\n\ttime: %0t, loop_cntr: %0d",$time,loop_cntr);
  end

endmodule
