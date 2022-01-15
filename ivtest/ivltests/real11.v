/*
* This tests the case that a delay value is a calculated real value.
*/
module main;

   real test;

   wire [3:0] Q;
   reg [3:0]  D;
   assign #(test/2.0) Q = D;

   initial begin
      test = 4.0;

      D = 1;
      #(test) if (Q !== 1) begin
	 $display("FAILED -- %0t: Q=%d, D=%d", $time, Q, D);
	 $finish;
      end

      D = 2;
      #(test/4) if (Q !== 1) begin
	 $display("FAILED -- %0t: Q=%d, D=%d", $time, Q, D);
	 $finish;
      end

      #(test/2) if (Q !== 2) begin
	 $display("FAILED -- %0t: Q=%d, D=%d", $time, Q, D);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
