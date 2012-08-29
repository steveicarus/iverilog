/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

// This example describes a 16x1 RAM that can be synthesized into
// a CLB ram in a Xilinx FPGA.

module ram16x1 (q, d, a, we, wclk);
   output q;
   input d;
   input [3:0] a;
   input we;
   input wclk;

   reg mem[15:0];

   assign q = mem[a];
   always @(posedge wclk) if (we) mem[a] = d;

endmodule /* ram16x1 */

module main;
   wire q;
   reg d;
   reg [3:0] a;
   reg we, wclk;

   ram16x1 r1 (q, d, a, we, wclk);

   initial begin
      $monitor("q = %b", q);
      d = 0;
      wclk = 0;
      a = 5;
      we = 1;
      #1 wclk = 1;
      #1 wclk = 0;
   end
endmodule /* main */
