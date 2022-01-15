/*
 * Copyright (c) 2001 Brendan J Simon <brendan.simon@bigpond.com>
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

// test case to show vector ordering bugs.

module  test;

reg [4:0] foo40;	// works great.

reg	[0:4] foo04;	// only works for time=0;
//reg	[4:0] foo04;

reg	[5:1] foo51;	// never works.
//reg	[4:0] foo51;

reg	[1:5] foo15;	// never works.
//reg	[4:0] foo15;

initial begin
	#102; $finish(0);
end

initial #1 begin
	foo40 = 0;
	foo04 = 0;
	foo51 = 0;
	foo15 = 0;
end

always #10 begin
	foo40 <= foo40 + 1;
	foo04 <= foo04 + 1;
	foo51 <= foo51 + 1;
	foo15 <= foo15 + 1;
end

always @(foo40) begin
	$write("foo40=%8d\n", foo40);
end

always @(foo04) begin
	$write("               foo04=%8d\n", foo04);
end

always @(foo51) begin
	$write("                              foo51=%8d\n", foo51);
end

always @(foo15) begin
	$write("                                             foo15=%8d\n", foo15);
end

endmodule
