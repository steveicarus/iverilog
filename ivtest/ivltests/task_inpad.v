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
 * The assignment to the input of a task port should pad with
 * zeros. It seems that certain Verilog bugs can cause this test to
 * fail.
 */

module test;
   task writeInto;
      input [31:0] x;
      begin
	 $display("x=%h", x);
	 if (x[31:10] !== 22'd0) begin
	    $display("FAILED -- x is %b", x);
	    $finish;
	 end
      end
   endtask

   reg [7:0] y;
   reg [31:0] y1;
   initial begin
      y1 = 512;
      y = 4;
      writeInto(y1);
      writeInto(y);
      $display("PASSED");
  end
endmodule
