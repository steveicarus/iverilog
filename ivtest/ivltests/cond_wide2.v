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

module foo;

   reg [2:0] cond;
   reg	     test;

   initial begin
      cond = 0;
      test = cond ? 1'b1 : 1'b0;

      if (test !== 1'b0) begin
	 $display("FAILED -- cond=%b, test=%b", cond, test);
	 $finish;
      end

      cond = 1;
      test = cond ? 1'b1 : 1'b0;

      if (test !== 1) begin
	 $display("FAILED -- cond=%b, test=%b", cond, test);
	 $finish;
      end

      cond = 2;
      test = cond ? 1'b1 : 1'b0;

      if (test !== 1) begin
	 $display("FAILED -- cond=%b, test=%b", cond, test);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule
