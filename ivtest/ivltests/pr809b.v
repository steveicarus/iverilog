module main;
   wire [1:0] a2, b2;
   wire [2:0] a3, b3;

   target #(.WA(2), .WB(2)) u1 (a2, b2);
   target #(.WA(3), .WB(3)) u2 (a3, b3);

   initial begin
      $display("u1.WA=%d, $bits(u1.A)=%d", u1.WA, $bits(u1.A));
      $display("u1.WB=%d, $bits(u1.A)=%d", u1.WB, $bits(u1.B));

      if ($bits(u1.A) != 2) begin
	 $display("FAILED -- $bits(u1.A) = %d", $bits(u1.A));
	 $finish;
      end

      if ($bits(u2.A) != 3) begin
	 $display("FAILED -- $bits(u2.A) = %d", $bits(u2.A));
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main

module target
   #(parameter WA = 4, parameter WB = 4)
    (input [WA-1:0] A, output [WB-1:0] B);

   assign B = A;

endmodule // target
