/*
 * Copyright (c) 2001 Steve Williams (steve@icarus.com)
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

/* Check that display prints the right signed value. */

module signed1();

   reg [7:0] x;
   reg signed [7:0] y;

   initial
     begin
	x = 8'b0000_0011;
	y = 8'b1111_1101;

	$display("x = %0d (should be 3)",x);
	$display("y = %0d (should be -3)",y);

	x = y;
	$display("x = %0d (should be 253)",x);


     end

endmodule
