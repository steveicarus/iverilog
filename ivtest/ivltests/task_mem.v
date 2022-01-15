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
 * This program tests the use of memories within tasks.
 */
module test;
   parameter addrsiz = 14;
   parameter ramsiz = 1 << addrsiz;

   task loadram;
      integer i, j;
      reg [15:0] memword;
      reg [15:0] tempram[0:(2*ramsiz)-1];
      begin
	 for (i = 0; i < 16; i = i + 2)
	    tempram[i] = i;

	 for (i = 0; i < 16; i = i + 2)
	   if (tempram[i] !== i) begin
	      $display("FAILED -- %m.tempram[%d] = %b", i, tempram[i]);
	      $finish;
	   end

	 $display("PASSED");
      end
   endtask // loadram

   initial loadram;

endmodule
