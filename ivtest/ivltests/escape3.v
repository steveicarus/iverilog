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

/*
 * This tests the rule in section 2.7.1:
 *   "Neither the leading backslash character nor the terminating
 *   white space is considered to be part of the identifier. There-
 *   fore, an escaped identifier \cpu3  is treated the same as a
 *   non escaped identifier cpu3."
 *
 * The cpu3 and \cpu3  notations are for the same object.
 */

module top;

   reg \cpu3 ;

   initial begin
      cpu3 = 1;
      $display("cpu3 == %b", \cpu3 );
      if (top.\cpu3  !== cpu3) begin
	 $display("FAILED -- top.\\cpu3 !== cpu3");
	 $finish;
      end

      if (\top .cpu3  !== \cpu3 ) begin
	 $display("FAILED -- \\top .cpu3 !== cpu3");
	 $finish;
      end

      if (top.\cpu3  !== 1) begin
	 $display("FAILED -- top.\\cpu3 !== 1");
	 $finish;
      end

      $display("PASSED");
   end
endmodule // top
