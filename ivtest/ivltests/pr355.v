/*
 * Copyright (c) 2001 Ted Bullen
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
// This code is released to Steve Williams for the Icarus Verilog compiler
// It can be used as desired !


// DEFINES
`define NRBITS 4	// Number of bits in each operand

// TOP MODULE
module testFunction();

   // SIGNAL DECLARATIONS
   reg				clock;
   reg [`NRBITS-1:0]		a_in;
   integer			a_integer;
   integer			myint;
   reg [`NRBITS:0]		cycle_count;	// Counts valid clock cycles

   // Initialize inputs
   initial begin
	  clock = 1;
	  cycle_count = 0;
      # (16*200+15) $display("PASSED");
      $finish;
   end

   // Generate the clock
   always #100 clock = ~clock;

   // Simulate
   always @(negedge clock) begin
	  cycle_count = cycle_count + 1;

	  // Create inputs between 0 and all 1s
	  a_in = cycle_count[`NRBITS-1:0];
	  myint = a_in;
	  $display("a_in = %d, myint = %d", a_in, myint);

	  // Convert the unsigned numbers to signed numbers
	  a_integer = short_to_int(a_in);


	  if (myint !== a_integer)
		 begin
			$display("ERROR ! %d !== %d", myint, a_integer);
			$stop;
		 end
   end

   // Function	to convert a reg of `NRBITS
   // bits to a signed integer
   function integer short_to_int;

	  input [`NRBITS-1:0]	x;
	  begin
		 short_to_int = x;
		 $display("\tshort_to_int(%b) = %b", x, short_to_int);
	  end
   endfunction
endmodule
