// Copyright (c) 2000 Stephen Williams (steve@icarus.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//

 // This tests DFF-like behavior. The clocked always block acts like a
 // DFF, and if the -Fsynth flag to ivl is used, actually generates an
 // LPM_FF device.

module main () ;

   reg clk;
   reg D, Q;

   always #10 clk = ~clk;
   always @(posedge clk) Q = D;

   initial begin
      clk = 0;
      D = 0;
      @(negedge clk)
         if (Q !== 1'b0)
             begin
	        $display("FAILED: %b !== %b", Q, D);
	        $finish;
             end

      D = 1;
      @(negedge clk)
         if (Q !== 1'b1)
            begin
	        $display("FAILED: %b !== %b", Q, D);
	        $finish;
            end

      D = 'bx;

      @(negedge clk)
         if (Q !== 1'bx)
            begin
	        $display("FAILED: %b !== %b", Q, D);
	        $finish;
            end

      D = 'bz;
      @(negedge clk)
         if (Q !== 1'bz)
            begin
	        $display("FAILED: %b !== %b", Q, D);
	        $finish;
            end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule
