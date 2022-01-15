/***********************************************************************
 *
 * Copyright (C) 2011 Adrian Wise
 *
 * This source code is free software; you can redistribute it
 * and/or modify it in source code form under the terms of the GNU
 * General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 ***********************************************************************
 *
 * This is a testbench exercising gate-level modelling of DTL gates,
 * distilled down (as a test-case) from a much larger design.
 * The gates can only pull down strongly to ground and have a weak
 * pull-up.
 *
 **********************************************************************/

`timescale 1 ns / 100 ps

module dtl_inv (op, in1);
   output op;
   input  in1;
   not (strong0, pull1) #16 not1 (op, in1);
endmodule // dtl_inv

module sr_latch (p, n);

   inout p;
   inout n;

   dtl_inv u_p1
     ( .in1 ( n ) ,
       .op  ( p ) );

   dtl_inv u_n1
     ( .in1 ( p ) ,
       .op  ( n ) );

endmodule // sr_latch

module dut (pp, nn);

   inout [1:0] pp;
   inout [1:0] nn;

   sr_latch u_l1 (pp[0], nn[0]);

endmodule // dut


module top;

   reg pass;
   reg x;

   wire [1:0] pp;
   wire [1:0] nn;

   dtl_inv u_pp0(.in1(~x), .op(pp[0]));
   dtl_inv u_nn0(.in1( x), .op(nn[0]));

   dut u_d1 (pp, nn);

   initial begin
      pass = 1'b1;
      x <= 2'd0;

      #100;

      $display("Expect: x = 0, pp = z0, nn = z1 p=0, n=1");
      $display("Actual: x = %b, pp = %b, nn = %b p=%b, n=%b", x, pp, nn,
               u_d1.u_l1.p, u_d1.u_l1.n);

      if (x !== 1'b0) begin
         $display("Failed: expected x to be 0, got %b", x);
         pass = 1'b0;
      end
      if (pp !== 2'bz0) begin
         $display("Failed: expected pp to be z0, got %b", pp);
         pass = 1'b0;
      end
      if (nn !== 2'bz1) begin
         $display("Failed: expected nn to be z0, got %b", nn);
         pass = 1'b0;
      end
      if (u_d1.u_l1.p !== 1'b0) begin
         $display("Failed: expected p to be 0, got %b", u_d1.u_l1.p);
         pass = 1'b0;
      end
      if (u_d1.u_l1.n !== 1'b1) begin
         $display("Failed: expected n to be 1, got %b", u_d1.u_l1.n);
         pass = 1'b0;
      end

      if (pass) $display("PASSED");

   end

endmodule // top
