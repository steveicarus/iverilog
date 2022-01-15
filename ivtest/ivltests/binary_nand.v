//
// Copyright (c) 2002 Stephen Williams
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
//  Binary ~& (nand) operator.


module main;

   reg A, B;
   reg result1;
   wire result2 = A ~& B;

   initial
     begin
	A = 0;
	B = 0;
	#1 result1 = A ~& B;
	if (result1 !== 1'b1) begin
	   $display("FAILED");
	   $finish;
	end
	if (result2 !== 1'b1) begin
	   $display("FAILED");
	   $finish;
	end

	A = 1;
	#1 result1 = A ~& B;
	if (result1 !== 1'b1) begin
	   $display("FAILED");
	   $finish;
	end
	if (result2 !== 1'b1) begin
	   $display("FAILED");
	   $finish;
	end

	B = 1;
	#1 result1 = A ~& B;
	if (result1 !== 1'b0) begin
	   $display("FAILED");
	   $finish;
	end
	if (result2 !== 1'b0) begin
	   $display("FAILED");
	   $finish;
	end

	A = 0;
	#1 result1 = A ~& B;
	if (result1 !== 1'b1) begin
	   $display("FAILED");
	   $finish;
	end
	if (result2 !== 1'b1) begin
	   $display("FAILED");
	   $finish;
	end

	$display("PASSED");
     end

endmodule // main
