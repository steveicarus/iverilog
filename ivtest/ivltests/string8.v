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

	reg [13:0] myReg14;
	reg [15:0] myReg16;

	initial begin
		$display("============================ myReg14 = 33*256+65");
		myReg14 = 33*256 + 65;
		$display(">|!A|");
		$display("*|%s|", myReg14);
		$display(">|!|");
		$display("*|%s|", myReg14[13:8]);
		$display("============================ myReg16 = 33*512+65*2");
		myReg16 = 33*512 + 65*2;
		$display(">|!A|");
		$display("*|%s|", myReg16[14:1]);
	end
endmodule
