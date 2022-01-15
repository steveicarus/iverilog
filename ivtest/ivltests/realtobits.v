/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
 * Icarus Verilog declares that $realtobits shall return a 64bit
 * number that is the normalized IEEE754 encoding of the real value.
 * Whatever it takes to get that, it does. It should be easy in
 * general, as most modern processors think in IEEE754 floating point
 * anyhow.
 */

module main;

   real val;

   initial begin
      val = 0.0;
      $display("val=%f (%h)", val, $realtobits(val));
      if ($realtobits(val) !== 64'h0000000000000000) begin
	 $display("FAILED");
	 $finish;
      end

      val = 1.0;
      $display("val=%f (%h)", val, $realtobits(val));
      if ($realtobits(val) !== 64'h3ff0000000000000) begin
	 $display("FAILED");
	 $finish;
      end

      val = 0.5;
      $display("val=%f (%h)", val, $realtobits(val));
      if ($realtobits(val) !== 64'h3fe0000000000000) begin
	 $display("FAILED");
	 $finish;
      end

      val = 1.5;
      $display("val=%f (%h)", val, $realtobits(val));
      if ($realtobits(val) !== 64'h3ff8000000000000) begin
	 $display("FAILED");
	 $finish;
      end

      val = 1.125;
      $display("val=%f (%h)", val, $realtobits(val));
      if ($realtobits(val) !== 64'h3ff2000000000000) begin
	 $display("FAILED");
	 $finish;
      end

      val = 2.6;
      $display("val=%f (%h)", val, $realtobits(val));
      if ($realtobits(val) !== 64'h4004cccccccccccd) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
