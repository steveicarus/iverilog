/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
 *
 * $Id: onehot16_tb.v,v 1.1 2003/03/31 01:35:05 stevewilliams Exp $
 */

/*
 * Exhaustive check of all the subtract results.
 */
module main;

   wire [15:0] out;
   reg [3:0] A;

   onehot16 dut(.out(out), .A(A));

   reg	     error = 0;
   integer   adx;

   initial begin
      A = 0;

      for (adx = 0 ;  adx < 16 ;  adx = adx + 1) begin
	 A = adx;
	 #1 $write("onehot(%b): %b", A, out);
	 if (out !== (1 << adx)) begin
	    $display(" ERROR");
	    error = 1;
	 end else begin
	    $display(" OK");
	 end

      end // for (adx = 0 ;  adx < 256 ;  adx = adx + 1)

      if (error == 0)
	$display("PASSED");
      else
	$display("FAILED");

   end // initial begin
endmodule // main
