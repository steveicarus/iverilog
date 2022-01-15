/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.will need a Picture Elements Binary Software
 *    License.
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
 * This program catches some glitches in the MUXZ that Icarus Verilog
 * uses to implement the ?: in structural cases.
 */
module main;

   parameter FOO = 0;

   initial begin
      if ((FOO < -255) || (FOO > 255)) begin
	 $display("FAILED -- foo=%d does not fall in the range?", FOO);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
