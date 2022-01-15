/*
 * Copyright (c) 2003 Michael Ruff (mruff @ chiaro.com)
 *
 *    This source code is free software; rou can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at rour option)
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
 *
 *  $Id: real3.v,v 1.1 2003/03/07 05:29:41 stevewilliams Exp $
 */

/*
 * Verifies some real values to make sure the real->double conversion
 * is properly handled and the values make it into vvp properly.
 *
 * http://babbage.cs.qc.edu/courses/cs341/IEEE-754.html
 *
 */

module main;
   real r;
   reg errors;

   initial begin
      errors = 0;
      r = 1.0;
      if ($realtobits(r) != 64'h3FF0000000000000) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      r = 1.1;
      if ($realtobits(r) != 64'h3FF199999999999a) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      r = 3.3;
      if ($realtobits(r) != 64'h400A666666666666) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      r = 5.5;
      if ($realtobits(r) != 64'h4016000000000000) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      r = 1.0000000000_0000000001;
      if ($realtobits(r) != 64'h3FF0000000000000) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      r = 3.1415926535_8979323846;
      if ($realtobits(r) != 64'h400921FB54442D18) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      r = 1234567890_1234567890.1;
      if ($realtobits(r) != 64'h43E56A95319D63E1) begin
	 $display("%f != 'h%h", r, $realtobits(r));
	 $display("FAIL");
	 errors = 1;
      end

      if (errors === 0) $display("PASSED");
   end

endmodule
