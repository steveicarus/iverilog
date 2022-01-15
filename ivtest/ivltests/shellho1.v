/*
 * Copyright (c) 2000 Mark Schellhorn <schellho@nortelnetworks.com>
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

/****************************************************************************

   The following code illustrates an apparent bug in the VCS v5.2 simulator.
   The check for whether or not the memory[0] array element was set correctly
   fails; yet the $display statement immediately after the check shows the
   memory[0] element actually has been set correctly.

   I've noted that if the $display and the check are reversed, the $display
   is incorrect but the check passes.

   This appears to be a problem with the simulator's internal scheduler.

***************************************************************************/


module bug_test;

reg [1:0] temp;

initial begin

 /********** First test. */

 $display("Running first test.\n");

 // Try setting memory array in other module using hierarchical
 // references & concatenation.
 temp = 2'h3;
 top.memory.memory[0] = {2'h0,temp};
 top.memory.memory[0] = {top.memory.memory[0],temp};

 // Check that setting was made correctly
 if (top.memory.memory[0] != {2{temp}}) begin
  $display("ERROR! top.memory.memory[0] failed to get");
  $display("set correctly!");
 end else begin
  $display("PASS! top.memory.memory[0] set correctly.");
 end

 // Display the value that was checked
 $display("top.memory.memory[0] = %h",top.memory.memory[0]);



 /********** Second test. */

 $display("\nRunning second test.\n");

 // Try setting memory array in other module using hierarchical
 // references & concatenation.
 temp = 2'h3;
 top.memory.memory[1] = {2'h0,temp};
 top.memory.memory[1] = {top.memory.memory[1],temp};

 // Display the value that will be checked
 $display("top.memory.memory[1] = %h",top.memory.memory[1]);

 // Check that setting was made correctly
 if (top.memory.memory[1] != {2{temp}}) begin
  $display("ERROR! top.memory.memory[1] failed to get");
  $display("set correctly!");
 end else begin
  $display("PASS! top.memory.memory[1] set correctly.");
 end

 // Display the value that was checked
 $display("top.memory.memory[1] = %h",top.memory.memory[1]);

 $finish(0);

end

endmodule

// Module containing a memory array
module memory;

reg [3:0] memory [0:1];

endmodule

// Module to instantiate test and memory modules
module top;

 bug_test bug_test();
 memory memory();

endmodule
