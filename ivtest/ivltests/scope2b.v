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
 * This test checks that the upwards search for a name stops at a
 * module boundary. In this example, the q variable in the instance
 * "inst" of the test module should be an implicit wire, even though
 * it is placed into the containing main scope that has a wire q in it.
 */

module test(p);
   output p;

   assign q = 1; // This should generate an error, q not defined
   assign p = q;

endmodule // test

module main;

   wire q = 0;
   wire sig;
   test inst(sig);

   initial begin
      #1 if (q !== 1'b0) begin
	 $display("FAILED -- main.q == %b", q);
	 $finish;
      end

      if (sig !== 1'b1) begin
	 $display("FAILED -- main.test.q == %b", sig);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
