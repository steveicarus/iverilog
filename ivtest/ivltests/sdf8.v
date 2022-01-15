`timescale 1ns / 1ps

`celldefine
// Description : 2 input XOR
module XOR20 (input A, input B, output Q);

   xor   (Q,B,A);

   specify
      (A => Q) = (1,1);
      (B => Q) = (1,1);
   endspecify

endmodule
`endcelldefine

`celldefine
module TIE_HIGH (output Q);
   buf(Q, 1'b1);
endmodule
`endcelldefine

module tb;

   reg a;
   wire b, q;
   XOR20 dut(.A(a), .B(b), .Q(q));
   TIE_HIGH src(.Q(b));

   initial begin
      $monitor($time,, "A=%b, B=%b, Q=%b", a, b, q);
      $sdf_annotate("ivltests/sdf8.sdf");

      #10 ;
      a = 1;
      #10 ;
      a = 0;
      #10 $finish(0);
   end

endmodule // tb
