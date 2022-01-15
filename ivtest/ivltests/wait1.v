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


module main;

   reg foo = 0;

   initial #10 foo = 1;

   initial #1 begin
      if (foo !== 1'b0) begin
	 $display("FAILED -- foo before wait is %b", foo);
	 $finish;
      end

      // This wait without a statement has caused a few bugs.
      wait (foo) ;

      if (foo !== 1'b1) begin
	 $display("FAILED -- foo after wait is %b", foo);
	 $finish;
      end

      if ($time != 10) begin
	 $display("FAILED -- $time after wait is %t", $time);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
