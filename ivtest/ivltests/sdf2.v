`celldefine
//`timescale 1ns / 1ps

// Description : 2 input XOR

module XOR20 (input A, input B, output Q);

   xor   (Q,B,A);

   specify
      (A => Q) = (1,1);
      (B => Q) = (1,1);
   endspecify

endmodule

`endcelldefine

module plug(input A, input B, output Q);

   XOR20 U1(.A(A), .B(B), .Q(Q));

endmodule // plug

module tb;

   reg a, b;
   wire q;
   plug dut(.A(a), .B(b), .Q(q));

   initial begin
      $monitor($time,, "A=%b, B=%b, Q=%b", a, b, q);
      $sdf_annotate("ivltests/sdf2.sdf");

      #10 ;
      a = 1;
      b = 1;
      #10 ;
      b = 0;
      #10 $finish(0);
   end

endmodule // tb
