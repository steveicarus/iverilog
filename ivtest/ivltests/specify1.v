/*
 * Copyright (c) 2000 Philips Semiconductors Stefan.Thiede@philips.com
 *
 *    This source code is free software; you can redistribute it
 *  Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

`timescale  1ns /  10ps
`celldefine

module MULT_8x8_f (P, A, B);

   parameter
      NA = 8,
      NB = 8,
      NP = 16,
      PX = 16'bx;

   input  [NA-1:0] A;
   input  [NB-1:0] B;
   output [NP-1:0] P;

   reg [NA-1:0] A_sign;
   reg [NB-1:0] B_sign;
   reg [NP-1:0] P_sign;
   reg Sign;

   wire [NA-1:0] AI;
   wire [NB-1:0] BI;

   // SDF constraint: set of buffers to allow the MIPDs annotation
   buf a0 (AI[0], A[0]);
   buf a1 (AI[1], A[1]);
   buf a2 (AI[2], A[2]);
   buf a3 (AI[3], A[3]);
   buf a4 (AI[4], A[4]);
   buf a5 (AI[5], A[5]);
   buf a6 (AI[6], A[6]);
   buf a7 (AI[7], A[7]);

   buf b0 (BI[0], B[0]);
   buf b1 (BI[1], B[1]);
   buf b2 (BI[2], B[2]);
   buf b3 (BI[3], B[3]);
   buf b4 (BI[4], B[4]);
   buf b5 (BI[5], B[5]);
   buf b6 (BI[6], B[6]);
   buf b7 (BI[7], B[7]);

   reg change_in;
   wire change_out;

   initial change_in = 0;

   buf #(0.01) bc (change_out, change_in);

   wire [NP-1:0] PI;

   // SDF constraint: P_sign cannot be used to drive directly buffers
   assign PI = P_sign;

   // SDF constraint: set of buffers to allow the output acceleration
   buf p0 (P[0], PI[0]);
   buf p1 (P[1], PI[1]);
   buf p2 (P[2], PI[2]);
   buf p3 (P[3], PI[3]);
   buf p4 (P[4], PI[4]);
   buf p5 (P[5], PI[5]);
   buf p6 (P[6], PI[6]);
   buf p7 (P[7], PI[7]);
   buf p8 (P[8], PI[8]);
   buf p9 (P[9], PI[9]);
   buf p10 (P[10], PI[10]);
   buf p11 (P[11], PI[11]);
   buf p12 (P[12], PI[12]);
   buf p13 (P[13], PI[13]);
   buf p14 (P[14], PI[14]);
   buf p15 (P[15], PI[15]);

   specify
      specparam
         th = 0.940,
         td = 4.930;

      // Pin-to-pin delay
      (A, B *> P) = (td, td, th, 0, th, 0);
   endspecify


endmodule
`endcelldefine
