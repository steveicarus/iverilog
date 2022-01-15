/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
 * This example catches the case of a unser defined function that is
 * a parameter to a system task.
 */

module main;

   function [15:0] sum;
      input [15:0] a;
      input [15:0] b;

      sum = a + b;
   endfunction // sum

   initial begin
      $display("%h = sum(%h, %h)", sum(3,5), 16'd3, 16'd5);
      $display("PASSED");
   end

endmodule // main
