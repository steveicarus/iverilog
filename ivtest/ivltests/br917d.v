
module main;

   localparam AMAX = 7;
   localparam SHIFT = 4;
   longint foo [AMAX:0];
   longint unsigned foo_u [AMAX:0];
   int idx;

   initial begin
      for (idx = 0 ; idx <= AMAX ; idx = idx+1) begin
	 foo[idx] = idx - SHIFT;
	 foo_u[idx] = idx;
      end

      for (idx = 0 ; idx <= AMAX ; idx = idx+1) begin
	 if (idx < SHIFT && foo[idx] > 0) begin
	    $display("FAIL -- foo[%0d] = %0d (not signed?)", idx, foo[idx]);
	    $finish;
	 end
	 if (foo[idx] != (idx-SHIFT)) begin
	    $display("FAIL -- foo[%0d] = %0d", idx, foo[idx]);
	    $finish;
	 end
	 if (foo_u[idx] != idx) begin
	    $display("FAIL -- foo_u[%0d] = %0d", idx, foo[idx]);
	    $finish;
	 end
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main
