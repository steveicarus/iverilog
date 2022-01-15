/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
 * This program combines two events with an event or. The tricky part
 * for the vvp target is that there is a mix of posedge and negedge
 * events.
 */

module ndFF ( nset, reset, Q );

   input  nset;            // clk   negedge set Q=1
   input  reset;           // reset posedge set Q=0
   output Q ;              // Q output
   reg    Q ;

   always @(negedge nset or posedge reset)
     begin
	if (nset ==1'b0)  Q  = 1'b1;
	else if (reset==1'b1) Q  = 1'b0;
     end

endmodule

module main;

   reg nset, reset;
   wire Q;

   ndFF dut(nset, reset, Q);

   initial begin
      #0 nset = 1;
      reset = 1;
      #1 nset = 0;
      #1 if (Q !== 1'b1) begin
	 $display("FAILED (a) nset=%b, reset=%b, Q=%b", nset, reset, Q);
	 $finish;
      end
      nset = 1;
      #1 if (Q !== 1'b1) begin
	 $display("FAILED (b) nset=%b, reset=%b, Q=%b", nset, reset, Q);
	 $finish;
      end
      reset = 0;
      #1 if (Q !== 1'b1) begin
	 $display("FAILED (c) nset=%b, reset=%b, Q=%b", nset, reset, Q);
	 $finish;
      end
      reset = 1;
      #1 if (Q !== 1'b0) begin
	 $display("FAILED (d) nset=%b, reset=%b, Q=%b", nset, reset, Q);
	 $finish;
      end
      $display("PASSED");

   end // initial begin

endmodule // main
