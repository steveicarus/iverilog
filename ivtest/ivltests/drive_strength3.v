`begin_keywords "1364-2005"
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

/*
 * This tests the behavior of drive strength attached to a buf device.
 * The assign of a reg to the bit should override the value and give a
 * well defined result.
 */
module main;

   wire bit;
   PULLDOWN pd(bit);

   reg	drv;
   assign bit = drv;

   initial begin
      drv = 0;
      #100 if (bit !== 1'b0) begin
	 $display("FAILED -- 0 bit = %b", bit);
	 $finish;
      end

      drv = 1;
      #100 if (bit !== 1'b1) begin
	 $display("FAILED -- 1 bit = %b", bit);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main


module PULLDOWN (O);

   output O;
   wire   A;

   pulldown (A);
   buf (weak0,weak1) #(1,1) (O,A);

endmodule
`end_keywords
