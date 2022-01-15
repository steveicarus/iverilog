/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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

/* Based on PR#564 */

module main( );


   parameter [7:0] forwards  = 8'b11110001;
   parameter [0:7] backwards = 8'b10001111;

   integer         i;

   initial begin
      for (i = 0 ;  i < 8 ;  i = i + 1) begin
	 $write("forwards[%0d] === %b, ", i, forwards[i]);
	 $display("backwards[%0d] === %b", i, backwards[i]);

	 if (forwards[i] !== backwards[i]) begin
	    $display("FAILED -- forwards[%0d] !== backwards[%0d]", i, i);
	    $finish;
	 end
      end

      $display("PASSED");
   end
endmodule // main
