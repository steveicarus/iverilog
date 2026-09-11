`timescale 1ns / 1ps

// Check that the (TIMESCALE ...) entry in the SDF header is honored. The SDF
// below expresses its delays in picoseconds while this module works in
// nanoseconds, so a 2000 unit IOPATH delay must be annotated as 2.0 ns. Before
// the timescale was honored the same file produced a 2000 ns delay.

`celldefine
module BUF1 (input A, output Q);
   buf (Q, A);
   specify
      (A => Q) = (1, 1);
   endspecify
endmodule
`endcelldefine

module top;

   reg  A;
   wire Q;

   BUF1 u1 (.A(A), .Q(Q));

   initial $sdf_annotate("ivltests/sdf_timescale.sdf", top);

   initial begin
      $monitor("%0t A=%b, Q=%b", $time, A, Q);
      A = 1'b0;
      #10 A = 1'b1;
      #10 A = 1'b0;
      #10 $finish(0);
   end

endmodule
