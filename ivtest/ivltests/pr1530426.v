module main;

   parameter N = 2**4;

   initial begin
      if (N != 16) begin
	 $display("FAILED -- N = %u (%h)", N, N);
	 $finish;
      end

      if (2**4 != 16) begin
	 $display("FAILED -- 2**16 = %u", 2**16);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endmodule // main
