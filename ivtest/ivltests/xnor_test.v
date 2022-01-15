/*
 * Copyright (c) 2000 Guy Hutchison (ghutchis@pacbell.net)
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

module xnor_test;

  reg onebit1, onebit2;
  reg [3:0] small1, small2;
  reg [15:0] large1, large2, large3, large4;
  reg	     fail;
  initial
    begin
      fail = 0;

      // single bit xnor testing
      if ((1'b0 ~^ 1'b1) === 1'b1) fail = 1;
      if ((1'b1 ^~ 1'b0) === 1'b1) fail = 1;
      if ((1'b0 ^~ 1'b0) === 1'b0) fail = 1;
      if ((1'b1 ~^ 1'b1) === 1'b0) fail = 1;

      // different sized operands (equality)
      for (small1=0; small1 < 15; small1=small1+1)
	begin
	  large1 = { 12'b0, small1 };

	  large2 = small1 ~^ large1;
	  if (large2 !== {16{1'b1}}) fail = 1;
	  large2 = large1 ^~ small1;
	  if (large2 !== {16{1'b1}}) fail = 1;
	end

      // random test
      // assumes +, &, |, and ~ work correctly
      for (large1 = 0; large1 < 1000; large1=large1+1)
	begin
	  large2 = large1 + 1511; // prime number

	  large3 = large1 ^~ large2;
	  large4 = (large1 & large2) | (~large1 & ~large2);

	  if (large3 !== large4)
	    begin
	      fail = 1;
	      $display ("Pattern failed: %h != %h", large3, large4);
	    end
	end // for (large1 = 0; large1 < 1000; large1=large1+1)

     if (fail)
	$display ("FAILED");
      else $display ("PASSED");
      $finish;
    end // initial begin

endmodule // xnor_test
