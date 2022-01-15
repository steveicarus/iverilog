// Copyright (c) 2015 CERN
// Maciej Suminski <maciej.suminski@cern.ch>
//
// This source code is free software; you can redistribute it
// and/or modify it in source code form under the terms of the GNU
// General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA


// Test for $ivlh_{rising,falling}_edge VPI functions
// (mostly used by the VHDL frontend).

module main;

   reg a, b;

   always @(a or b) begin
      if ($ivlh_rising_edge(a))
	$display("%0t: rising_edge(a)", $time);
      if ($ivlh_falling_edge(a))
	$display("%0t: falling_edge(a)", $time);

      if ($ivlh_rising_edge(b))
	$display("%0t: rising_edge(b)", $time);
      if ($ivlh_falling_edge(b))
	$display("%0t: falling_edge(b)", $time);
   end

   initial begin
      #1 a <= 1;
      #1 b <= 1;

      #1 a <= 0;
      #1 b <= 0;

      #1 a <= 0;    // nothing should be detected
      #1 b <= 0;

      #1 a <= 1;
      #1 b <= 1;

      #1 a <= 1;    // nothing should be detected
      #1 b <= 1;

      #1 a <= 0;
      #1 b <= 0;
      #1 $finish(0);
   end

endmodule // main
