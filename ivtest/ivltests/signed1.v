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
 * This program tests that signed registers are compared using signed
 * arithmetic.
 */
module main;

   reg signed [7:0] vala;
   reg signed [7:0] valb;
   reg        [7:0] valc;

   initial begin
      vala = -1;
      valb = +1;
      valc = -1;

      if (vala >= valb) begin
	 $display("FAILED -- vala(%b) is >= valb(%b)", vala, valb);
	 $finish;
      end

      if (vala > valb) begin
	 $display("FAILED -- vala(%b) is > valb(%b)", vala, valb);
	 $finish;
      end

      if (valb <= vala) begin
	 $display("FAILED -- valb(%b) is <= vala(%b)", valb, vala);
	 $finish;
      end

      if (valb < vala) begin
	 $display("FAILED -- valb(%b) is < vala(%b)", valb, vala);
	 $finish;
      end

      if (valc <= valb) begin
	 $display("FAILED -- valc(%b) is not > valb(%b)", valc, valb);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
