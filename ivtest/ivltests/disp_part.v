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
 * The output from this program should be:
 *   1001
 *   0100
 *   0010
 *   1001
 *   1100
 */

module main;

   reg [7:0] foo;

   initial begin
      foo = 8'b11001001;
      $display("%b", foo[3:0]);
      $display("%b", foo[4:1]);
      $display("%b", foo[5:2]);
      $display("%b", foo[6:3]);
      $display("%b", foo[7:4]);
   end

endmodule // main
