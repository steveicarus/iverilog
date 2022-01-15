//
// Copyright (c) 2001 Steve Williams <steve@icarus.com>
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//

/*
 * The $depost system task takes the inputs a depositible object and a
 * value to deposit. The $deposit works like a blocking assignment, so
 * the target takes the value right away.
 *
 * This example tests the $deposit on a wire object. What that means is
 * that the wire takes on the deposited value, but that value doesn't
 * stick if its normal input changes.
 */
module main ;

   reg in;
   wire test = in;

   initial begin
      in = 1'b0;
      #1 if (test !== 1'b0) begin
	 $display("FAILED -- test starts out as %b", test);
	 //$finish;
      end

      $deposit(test, 1'b1);

      #1 if (test !== 1'b1) begin
	 $display("FAILED -- test after deposit is %b", test);
	 $finish;
      end

      in = 1'bz;

      #1 if (test !== 1'bz) begin
	 $display("FAILED -- test after input is %b", test);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
