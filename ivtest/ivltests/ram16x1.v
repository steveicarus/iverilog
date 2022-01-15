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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
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
      wclk = 0;
      we = 1;
      for (a = 0 ; a < 4'hf ;  a = a + 1) begin
	 d = a[0];
	 #1 wclk = 1;
	 #1 wclk = 0;
	 $display("r1[%h] == %b", a, q);
      end

      for (a = 0 ; a < 4'hf ;  a = a + 1)
	 #1 if (q !== a[0]) begin
	    $display("FAILED -- mem[%h] !== %b", a, a[0]);
	    $finish;
	 end

      $display("PASSED");
   end
endmodule /* main */
