/*
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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

// $Id: many_drivers.v,v 1.2 2001/07/21 02:30:44 stevewilliams Exp $
// $Log: many_drivers.v,v $
// Revision 1.2  2001/07/21 02:30:44  stevewilliams
//  Get the expected blended values right.
//
// Revision 1.1  2001/07/18 01:22:26  sib4
// test for nets with many drivers
//

module test;

   reg [66:0]  in;
   wire        out;
   buf (out, in[ 0]);
   buf (out, in[ 1]);
   buf (out, in[ 2]);
   buf (out, in[ 3]);
   buf (out, in[ 4]);
   buf (out, in[ 5]);
   buf (out, in[ 6]);
   buf (out, in[ 7]);
   buf (out, in[ 8]);
   buf (out, in[ 9]);
   buf (out, in[10]);
   buf (out, in[11]);
   buf (out, in[12]);
   buf (out, in[13]);
   buf (out, in[14]);
   buf (out, in[15]);
   buf (out, in[16]);
   buf (out, in[17]);
   buf (out, in[18]);
   buf (out, in[19]);
   buf (out, in[20]);
   buf (out, in[21]);
   buf (out, in[22]);
   buf (out, in[23]);
   buf (out, in[24]);
   buf (out, in[25]);
   buf (out, in[26]);
   buf (out, in[27]);
   buf (out, in[28]);
   buf (out, in[29]);
   buf (out, in[30]);
   buf (out, in[31]);
   buf (out, in[32]);
   buf (out, in[33]);
   buf (out, in[34]);
   buf (out, in[35]);
   buf (out, in[36]);
   buf (out, in[37]);
   buf (out, in[38]);
   buf (out, in[39]);
   buf (out, in[40]);
   buf (out, in[41]);
   buf (out, in[42]);
   buf (out, in[43]);
   buf (out, in[44]);
   buf (out, in[45]);
   buf (out, in[46]);
   buf (out, in[47]);
   buf (out, in[48]);
   buf (out, in[49]);
   buf (out, in[50]);
   buf (out, in[51]);
   buf (out, in[52]);
   buf (out, in[53]);
   buf (out, in[54]);
   buf (out, in[55]);
   buf (out, in[56]);
   buf (out, in[57]);
   buf (out, in[58]);
   buf (out, in[59]);
   buf (out, in[60]);
   buf (out, in[61]);
   buf (out, in[62]);
   buf (out, in[63]);
   buf (out, in[64]);
   buf (out, in[65]);
   buf (out, in[66]);

   reg	       err;

   // Verilog-XL yields out=x for all but the first two

   initial
     begin
	err = 0;

	in =  67'b0;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'b0) err = 1;

	in = ~67'b0;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'b1) err = 1;

	in =  67'bz;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in =  67'bx;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in =  67'h 5_55555555_55555555;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in = ~67'h 5_55555555_55555555;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in =  67'h 0_xxxxxxxx_00000000;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in = ~67'h 0_xxxxxxxx_00000000;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in =  67'h x_xxxxxxxx_00000000;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in = ~67'h x_xxxxxxxx_00000000;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in =  67'h x_55555555_55555555;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in = ~67'h x_55555555_55555555;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in =  67'h 1_ffffxxxx_00000000;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	in = ~67'h 1_ffffxxxx_00000000;
	#1 $display("in=%b out=%b", in, out);
	if (out!==1'bx) err = 1;

	if (err)
	  $display("FAILED");
	else
	  $display("PASSED");
	$finish;
     end

endmodule
