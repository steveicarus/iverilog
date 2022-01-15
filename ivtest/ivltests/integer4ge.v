/*
 * integer4ge - a verilog test for integer greater-or-equal conditional >=
 *
 * Copyright (C) 1999 Stephen G. Tell
 * Portions inspired by qmark.v by Steven Wilson (stevew@home.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */

module integer4ge;

   integer a;
   integer b;
   reg	   error;

   initial begin
      error = 0;

      a = 1;
      if(a >= 2) begin
	 $display("FAILED 1 >= 2");
	 error = 1;
      end

      a = 2;
      if(a >= 2) begin
	 b = 1;
      end else begin
	 $display("FAILED 2 >= 2");
	 error = 1;
      end

      a = 3;
      if(a >= 2) begin
	 b = 1;
      end else begin
	 $display("FAILED 3 >= 2");
	 error = 1;
      end


      b = 0;
      for(a = 10; a >= 5; a = a - 1) begin
	 b = b + a;
      end

      if(b != 45) begin
	 $display("FAILED forloop b=%d expected 45", b);
	 error = 1;
      end

      if(error == 0)
	 $display("PASSED");
      $finish;

   end // initial begin

endmodule
