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
// $Id: memidxrng.v,v 1.1 2001/09/29 05:03:41 sib4 Exp $
// $Log: memidxrng.v,v $
// Revision 1.1  2001/09/29 05:03:41  sib4
// add memidxrng.v: memory address range check
//

module memidxrng;

   reg mem[12:2];

   reg [7:0] i;
   integer errs = 0;

   initial
     begin
	for (i=0; i<255; i=i+1) mem[i] <= ^i;
	#1;
	for (i=0; i<17; i=i+1)
	  $display("mem[%d] = %b \%b", i, mem[i], ^i);
`ifdef SUPPORT_CONST_OUT_OF_RANGE_IN_IVTEST
	if (mem[13] !== 1'bx)
	  begin
	     $display("FAILED: mem[13] = %b, expect x", mem[14]);
	     errs = errs + 1;
	  end
	if (mem[1] !== 1'bx)
	  begin
	     $display("FAILED: mem[1] = %b, expect x", mem[1]);
	     errs = errs + 1;
	  end
`endif
	if (errs===0)
	  $display("PASSED");
	$finish;
     end

endmodule
