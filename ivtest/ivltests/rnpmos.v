/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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

   wire no, po;
   reg	d, c;

   rnmos n (no, d, c);
   rpmos p (po, d, c);

   initial begin
      c = 0;
      d = 0;

      #1 if (no !== 1'bz) begin
	 $display("FAILED -- n (%b, %b, %b)", no, d, c);
	 $finish;
      end

      if (po !== 1'b0) begin
	 $display("FAILED -- p (%b, %b, %b)", po, d, c);
	 $finish;
      end

      d = 1;

      #1 if (no !== 1'bz) begin
	 $display("FAILED -- n (%b, %b, %b)", no, d, c);
	 $finish;
      end

      if (po !== 1'b1) begin
	 $display("FAILED -- p (%b, %b, %b)", po, d, c);
	 $finish;
      end

      c = 1;

      #1 if (no !== 1'b1) begin
	 $display("FAILED -- n (%b, %b, %b)", no, d, c);
	 $finish;
      end

      if (po !== 1'bz) begin
	 $display("FAILED -- p (%b, %b, %b)", po, d, c);
	 $finish;
      end

      d = 0;

      #1 if (no !== 1'b0) begin
	 $display("FAILED -- n (%b, %b, %b)", no, d, c);
	 $finish;
      end

      if (po !== 1'bz) begin
	 $display("FAILED -- p (%b, %b, %b)", po, d, c);
	 $finish;
      end

      $display("PASSED");

   end

endmodule // main
