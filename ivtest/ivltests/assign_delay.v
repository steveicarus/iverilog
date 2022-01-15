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
 * This test captures the essence of PR#40, namely the possibility
 * that the continuous assign from a reg to a wire may have a delay.
 */
module test;
   wire a;
   reg b;

   assign #10 a = b;

   initial begin
        b = 0;
        # 20 b = 1;
        # 5 if (a !== 0) begin
            $display("FAILED -- a is %b", a);
            $finish;
        end

        # 6 if (a !== 1) begin
            $display("FAILED -- a is %b, should be 1", a);
            $finish;
        end

        $display("PASSED");
    end
endmodule
