//
// Copyright (c) 2000 Stephen Williams (steve@icarus.com
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

/*
 * This test catches some rather tricky symbol binding issues.
 * Hierarchical binding gets tricky when tasks reach outside their
 * own scope to find things.
 */

module glob(fake);
   input fake;

   reg var1;
   reg var2;
endmodule // glob

module main;

   /* This task reaches into the U1 module instance to set some
      registers. This is tricky because task definitions are
      elaborated early, probably before the modules are elaborated. */

   task test;
      begin
	 U1.var1 = 1'b0;
	 U1.var2 = 1'b1;
      end
   endtask // test

   glob U1 (fake);

   initial begin
      test;
      if (U1.var1 !== 1'b0) begin
	 $display("FAILED -- globvar1 == %b", U1.var1);
	 $finish;
      end
      if (U1.var2 !== 1'b1) begin
	 $display("FAILED -- globvar2 == %b", U1.var2);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
