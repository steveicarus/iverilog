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
 * This program tests the ability to take bit and part selects
 * of parameters. This is actually not legal in Verilog, but
 * Icarus Verilog supports it anyhow, as do many (most?) other
 * Verilog compilers.
 */

module main;

   parameter vec = 16'b0000_1001_0111_1010;

   initial begin
      if (vec[0] !== 0) begin
	 $display("FAILED -- %b[0] !== 0", vec);
	 $finish;
      end

      if (vec[1] !== 1) begin
	 $display("FAILED -- %b[1] !== 1", vec);
	 $finish;
      end

      if (vec[3:1] !== 3'b101) begin
	 $display("FAILED -- %b[3:1] !== b101", vec);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
