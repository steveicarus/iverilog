/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
 * This program checks that a function execution that includes part
 * selects properly evaluates expressions. This is inspired by PR#95.
 */
module main;

   wire [3:0]   a = 4'h1;
   wire [3:0]   b = 4'h3;
   reg [1:0]    got1, got2;
   reg [7:0]    line;

   initial
     begin
        line = 8'h30;

        #1; // Need some delay for the assignments to run.
	got1 = { (b[3:0] == line[7:4]), (a[3:0] == line[3:0]) };
	got2 = test(a, b, line);

	$display("a=%b, b=%b, line=%b, got1=%b, got2=%b",
		 a, b, line, got1, got2);

	if (got1 !== 2'b10) begin
	   $display("FAILED  -- got1 is wrong: %b !== 2'b10", got1);
	   $finish;
	end

	if (got1 !== got2) begin
	   $display("FAILED  -- got2 is incorrect: %b !== %b", got1, got2);
	   $finish;
	end

	$display("PASSED");
        $finish;
     end

   function [1:0] test;
      input [3:0] a, b;
      input [7:0] line;
      test = { (b == line[7:4]), (a[3:0] == line[3:0]) };

   endfunction // test


endmodule // main
