
program main;

   int sum;
   logic idx; // Use this to test scope;
   initial begin
      sum = 0;
      idx = 1'bx;
      for (int idx = 0 ; idx < 8 ; idx += 1) begin
	 sum += idx;
      end

      if (sum != 28) begin
	 $display("FAILED -- sum=%0d", sum);
	 $finish;
      end

      if (idx !== 1'bx) begin
	 $display("FAILED -- idx in upper scope became %b", idx);
	 $finish;
      end

      $display("PASSED");
   end // initial begin

endprogram
