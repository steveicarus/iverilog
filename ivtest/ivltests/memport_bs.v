`ifdef __ICARUS__
  `define SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
`endif

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
// $Id: memport_bs.v,v 1.1 2001/10/13 03:35:01 sib4 Exp $
// $Log: memport_bs.v,v $
// Revision 1.1  2001/10/13 03:35:01  sib4
// PR#303 memport_bs.v
//

module pr303;

   reg [3:0]  mem [2:5];

`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
   wire [3:0] m1 = mem[1];
`else
   wire [3:0] m1 = 4'bxxxx;
`endif
   wire [3:0] m2 = mem[2];
   wire [3:0] m3 = mem[3];
   wire [3:0] m4 = mem[4];
   wire [3:0] m5 = mem[5];
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
   wire [3:0] m6 = mem[6];
`else
   wire [3:0] m6 = 4'bxxxx;
`endif

   reg [2:0]  a;
   reg [3:0]  e;

   initial
     begin
	e = 0;
	for (a=0; a<7; a=a+1) mem[a] <= a;
	#1;
	if (   m1  !== 4'hx) begin e=e+1; $display("FAILED    m1=%b",     m1 ); end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
	if (mem[1] !== 4'hx) begin e=e+1; $display("FAILED mem[1]=%b", mem[1]); end
`endif
	if (   m2  !== 4'h2) begin e=e+1; $display("FAILED    m2=%b",     m2 ); end
	if (mem[2] !== 4'h2) begin e=e+1; $display("FAILED mem[2]=%b", mem[2]); end
	if (   m3  !== 4'h3) begin e=e+1; $display("FAILED    m3=%b",     m3 ); end
	if (mem[3] !== 4'h3) begin e=e+1; $display("FAILED mem[3]=%b", mem[3]); end
	if (   m4  !== 4'h4) begin e=e+1; $display("FAILED    m4=%b",     m4 ); end
	if (mem[4] !== 4'h4) begin e=e+1; $display("FAILED mem[4]=%b", mem[4]); end
	if (   m5  !== 4'h5) begin e=e+1; $display("FAILED    m5=%b",     m5 ); end
	if (mem[5] !== 4'h5) begin e=e+1; $display("FAILED mem[5]=%b", mem[5]); end
	if (   m6  !== 4'hx) begin e=e+1; $display("FAILED    m6=%b",     m6 ); end
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
	if (mem[6] !== 4'hx) begin e=e+1; $display("FAILED mem[6]=%b", mem[6]); end
`endif
	if (e===0) $display("PASSED");
     end
endmodule
