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
 */

module main;

   reg [3:0] foo;

   initial begin
      foo = 0;
      foo <= 1;
      // This will display 1 at time=1
      $test_next_sim_time(foo);
      $strobe("foo should be 0: %d", foo);
      #1 foo <= #4 2;
      $strobe("foo should be 1: %d", foo);
      $test_next_sim_time(foo);
      #5 $display("foo is finally %d", foo);
   end

endmodule // main
