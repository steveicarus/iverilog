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

//`define DEBUG

`define BUG_FIX

module drive_strength;

  // strength values (append 1/0 to each):
  // supply -> strong -> pull -> weak -> highz

/*
 *   Strength Value Table
 *   1-->  supply | strong | pull | weak | highz
 *  supply    x   |   0    |  0   |  0   |   0
 *  strong    1   |   x    |  0   |  0   |   0
 *   pull     1   |   1    |  x   |  0   |   0
 *   weak     1   |   1    |  1   |  x   |   0
 *  highz     1   |   1    |  1   |  1   |   z
 */

  wire su1su0, su1st0, su1pu0, su1we0, su1hz0;
  wire st1su0, st1st0, st1pu0, st1we0, st1hz0;
  wire pu1su0, pu1st0, pu1pu0, pu1we0, pu1hz0;
  wire we1su0, we1st0, we1pu0, we1we0, we1hz0;
  wire hz1su0, hz1st0, hz1pu0, hz1we0, hz1hz0;

  /* supply assignments */
  assign (supply1, supply0) su1su0 = 1'b1;
  assign (supply1, supply0) su1st0 = 1'b1;
  assign (supply1, supply0) su1pu0 = 1'b1;
  assign (supply1, supply0) su1we0 = 1'b1;
  assign (supply1, supply0) su1hz0 = 1'b1;

   /* strong assignments */
  assign (strong1, strong0) st1su0 = 1'b1;
  assign (strong1, strong0) st1st0 = 1'b1;
  assign (strong1, strong0) st1pu0 = 1'b1;
  assign (strong1, strong0) st1we0 = 1'b1;
  assign (strong1, strong0) st1hz0 = 1'b1;

   /* pull assignments */
  assign (pull1, pull0) pu1su0 = 1'b1;
  assign (pull1, pull0) pu1st0 = 1'b1;
  assign (pull1, pull0) pu1pu0 = 1'b1;
  assign (pull1, pull0) pu1we0 = 1'b1;
  assign (pull1, pull0) pu1hz0 = 1'b1;

   /* weak assignments */
  assign (weak1, weak0) we1su0 = 1'b1;
  assign (weak1, weak0) we1st0 = 1'b1;
  assign (weak1, weak0) we1pu0 = 1'b1;
  assign (weak1, weak0) we1we0 = 1'b1;
  assign (weak1, weak0) we1hz0 = 1'b1;

   /* highz assignments */
  assign (highz1, strong0) hz1su0 = 1'b1;
  assign (highz1, strong0) hz1st0 = 1'b1;
  assign (highz1, strong0) hz1pu0 = 1'b1;
  assign (highz1, strong0) hz1we0 = 1'b1;
  assign (highz1, strong0) hz1hz0 = 1'b1;

  /* supply assignments */
  assign (supply1, supply0) su1su0 = 1'b0;
  assign (supply1, supply0) st1su0 = 1'b0;
  assign (supply1, supply0) pu1su0 = 1'b0;
  assign (supply1, supply0) we1su0 = 1'b0;
  assign (supply1, supply0) hz1su0 = 1'b0;

  /* strong assignments */
  assign (strong1, strong0) su1st0 = 1'b0;
  assign (strong1, strong0) st1st0 = 1'b0;
  assign (strong1, strong0) pu1st0 = 1'b0;
  assign (strong1, strong0) we1st0 = 1'b0;
  assign (strong1, strong0) hz1st0 = 1'b0;

  /* pull assignments */
  assign (pull1, pull0) su1pu0 = 1'b0;
  assign (pull1, pull0) st1pu0 = 1'b0;
  assign (pull1, pull0) pu1pu0 = 1'b0;
  assign (pull1, pull0) we1pu0 = 1'b0;
  assign (pull1, pull0) hz1pu0 = 1'b0;

  /* weak assignments */
  assign (weak1, weak0) su1we0 = 1'b0;
  assign (weak1, weak0) st1we0 = 1'b0;
  assign (weak1, weak0) pu1we0 = 1'b0;
  assign (weak1, weak0) we1we0 = 1'b0;
  assign (weak1, weak0) hz1we0 = 1'b0;

  /* highz assignments */
  assign (strong1, highz0) su1hz0 = 1'b0;
  assign (strong1, highz0) st1hz0 = 1'b0;
  assign (strong1, highz0) pu1hz0 = 1'b0;
  assign (strong1, highz0) we1hz0 = 1'b0;
  assign (strong1, highz0) hz1hz0 = 1'b0;

  initial
    begin
`ifdef DEBUG
      $dumpfile ("verilog.dump");
      $dumpvars (0, drive_strength);
`endif

      /* check all values for 1/x/0 */
      #1;	// Give things a chance to evaluate!!!
      if ((su1su0 !== 1'bx) ||
	  (su1st0 !== 1'b1) ||
	  (su1pu0 !== 1'b1) ||
	  (su1we0 !== 1'b1) ||
	  (su1hz0 !== 1'b1) ||
          (st1su0 !== 1'b0) ||
	  (st1st0 !== 1'bx) ||
	  (st1pu0 !== 1'b1) ||
	  (st1we0 !== 1'b1) ||
	  (st1hz0 !== 1'b1) ||
          (pu1su0 !== 1'b0) ||
	  (pu1st0 !== 1'b0) ||
	  (pu1pu0 !== 1'bx) ||
	  (pu1we0 !== 1'b1) ||
	  (pu1hz0 !== 1'b1) ||
          (we1su0 !== 1'b0) ||
	  (we1st0 !== 1'b0) ||
	  (we1pu0 !== 1'b0) ||
	  (we1we0 !== 1'bx) ||
	  (we1hz0 !== 1'b1) ||
          (hz1su0 !== 1'b0) ||
	  (hz1st0 !== 1'b0) ||
	  (hz1pu0 !== 1'b0) ||
	  (hz1we0 !== 1'b0) ||
	  (hz1hz0 !== 1'bz))
	$display ("FAILED - drive_strength");
      else
	$display ("PASSED");

      #10;
      $finish;
    end // initial begin

`ifdef BUG_FIX
  reg bug_fix;

  initial
    begin
      bug_fix = 0;
      #2;
      bug_fix = 1;
      #2;
      bug_fix = 0;
    end
`endif // ifdef BUG_FIX

endmodule
