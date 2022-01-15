`celldefine
//`timescale 1ns / 1ps

// Description : 2 input XOR

module XOR20 (input A, input B, output Q);

   xor   (Q,B,A);

   initial $sdf_annotate("ivltests/sdf3.sdf");

   specify
      (A => Q) = (1,1);
      (B => Q) = (1,1);
   endspecify

endmodule

`endcelldefine

module tb;

   reg a, b;
   wire q;
   XOR20 dut(.A(a), .B(b), .Q(q));

   initial begin
      $monitor($time,, "A=%b, B=%b, Q=%b", a, b, q);

      #10 ;
      a = 1;
      b = 1;
      #10 ;
      b = 0;
      #10 $finish(0);
   end

endmodule // tb
