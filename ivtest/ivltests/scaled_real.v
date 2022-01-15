module main;

   initial begin
      if (1.0a != 1.0e-18) begin
	 $display("FAILED -- 1.0a = %0.19g", 1.0a);
	 $finish;
      end
      if (1.0f != 1.0e-15) begin
	 $display("FAILED -- 1.0f = %0.19g", 1.0f);
	 $finish;
      end
      if (1.0p != 1.0e-12) begin
	 $display("FAILED -- 1.0p = %0.19g", 1.0p);
	 $finish;
      end
      if (1.0n != 1.0e-9) begin
	 $display("FAILED -- 1.0n = %0.19g", 1.0n);
	 $finish;
      end
      if (1.0u != 1.0e-6) begin
	 $display("FAILED - 1.0u = %0.19g", 1.0u);
	 $finish;
      end
      if (1.0m != 1.0e-3) begin
	 $display("FAILED -- 1.0m = %0.19g", 1.0m);
	 $finish;
      end
      if (1.0k != 1.0e3) begin
	 $display("FAILED -- 1.0k = %0.19g", 1.0k);
	 $finish;
      end
      if (1.0K != 1000.0) begin
	 $display("FAILED -- 1.0K = %f", 1.0K);
	 $finish;
      end
      if (1.0M != 1_000_000.0) begin
	 $display("FAILED -- 1.0M = %f", 1.0M);
	 $finish;
      end
      if (1.0G != 1_000_000_000.0) begin
	 $display("FAILED -- 1.0G = %f", 1.0G);
	 $finish;
      end
      if (1.0T != 1_000_000_000_000.0) begin
	 $display("FAILED -- 1.0T = %f", 1.0T);
	 $finish;
      end
      $display("PASSED");
   end // initial begin

endmodule // main
