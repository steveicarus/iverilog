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

module main;

   reg [3:0] test;

   initial begin
/*    A zero count repeat by it self is not allowed by the standard,
      test = {0{1'b1}};
      if (test !== 4'b0000) begin
	 $display("FAILED -- {0{1'b1} == %b", test);
	 $finish;
      end

      but it can be used in a valid concatenation (1364-2005). */
      test = {{0{1'b1}}, 1'b0};
      if (test !== 4'b0000) begin
	 $display("FAILED -- {0{1'b1} == %b", test);
	 $finish;
      end

      test = {1{1'b1}};
      if (test !== 4'b0001) begin
	 $display("FAILED -- {1{1'b1} == %b", test);
	 $finish;
      end

      test = {2{1'b1}};
      if (test !== 4'b0011) begin
	 $display("FAILED -- {2{1'b1} == %b", test);
	 $finish;
      end

      test = {3{1'b1}};
      if (test !== 4'b0111) begin
	 $display("FAILED -- {3{1'b1} == %b", test);
	 $finish;
      end

      test = {4{1'b1}};
      if (test !== 4'b1111) begin
	 $display("FAILED -- {4{1'b1} == %b", test);
	 $finish;
      end

      test = {5{1'b1}};
      if (test !== 4'b1111) begin
	 $display("FAILED -- {5{1'b1} == %b", test);
	 $finish;
      end

      $display("PASSED");

   end // initial begin

endmodule // main
