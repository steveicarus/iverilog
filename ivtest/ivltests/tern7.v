`begin_keywords "1364-2005"
/*
 * Copyright (c) 2005 Stephen Williams (steve@icarus.com)
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

/* tern7.v
 * This tests types.
 */
module main;

   reg b, c, d, e;
   wire a = b ? c : (d&e);

   reg [4:0] tmp;
   reg	     ref;
   initial begin
      // Do an exaustive scan of the possible values.
      for (tmp = 0 ;  tmp < 16 ;  tmp = tmp + 1) begin
	 b <= tmp[0];
	 c <= tmp[1];
	 d <= tmp[2];
	 e <= tmp[3];
	 ref = tmp[0] ? tmp[1] : (tmp[2]&tmp[3]);

	 #1 if (ref !== a) begin
	    $display("FAILED -- a=%b, b=%b, c=%b, d=%b, e=%b",
		     a, b, c, d, e);
	    $finish;
	 end
      end // for (tmp = 0 ;  tmp < 16 ;  tmp = tmp + 1)

      b <= 0;
      c <= 1;
      d <= 1;
      e <= 0;
      #1 if (a !== 1'b0) begin
	 $display("FAILED (1)");
	 $finish;
      end

      e <= 1;
      #1 if (a !== 1'b1) begin
	 $display("FAILED (2)");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
`end_keywords
