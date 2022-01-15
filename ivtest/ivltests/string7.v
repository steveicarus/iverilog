/*
 * Copyright (c) 2002 Tom Verbeure
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

	integer myInt;
	reg [39:0] myReg40;
	reg [0:39] myReg40r;
	reg [0:38] myReg39r;
	reg [13:0] myReg14;
	reg [7:0] myReg8;

	initial begin
		$display("============================ myReg8 = 65");
		myReg8 = 65;
		$display(">|A|");
		$display("*|%s|", myReg8);

		$display("============================ myReg40 = \"12345\"");
		myReg40 = "12345";

		$display(">|12345|");
		$display("*|%s|", myReg40);
		$display(">|5|");
		$display("*|%s|", myReg40[7:0]);

		$display("============================ myReg40r = \"12345\"");
		myReg40r = "12345";

		$display(">|12345|");
		$display("*|%s|", myReg40r);
		$display(">|1|");
		$display("*|%s|", myReg40r[0:7]);

		$display("============================ myReg39r = \"12345\"");
		myReg39r = "12345";

		$display(">|12345|");
		$display("*|%s|", myReg39r);
		$display(">|b|");
		$display("*|%s|", myReg39r[0:7]);

		$display("============================ myReg14 = 65");
		myReg14 = 65;

		$display(">| A|");
		$display("*|%s|", myReg14);

		$display("============================ myReg14 = 33*356+65");
		myReg14 = 33*256 + 65;
		$display(">|!A|");
		$display("*|%s|", myReg14);

		$display("============================ myInt = 65");
		myInt = 65;

		$display(">|   A|");
		$display("*|%s|", myInt);
	end
endmodule
