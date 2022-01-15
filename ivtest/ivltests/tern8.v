/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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

/* tern8.v
 * This tests types.
 */
module main;

   reg b;
   real c, d;
   wire real a = b ? c : d;

   initial begin

      b <= 0;
      c <= 1.0;
      d <= 2.0;
      #1 if (a != 2.0) begin
	 $display("FAILED (1)");
	 $finish;
      end

      b <= 1;
      #1 if (a != 1.0) begin
	 $display("FAILED (2)");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
