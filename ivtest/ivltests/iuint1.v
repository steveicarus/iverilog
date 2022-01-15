module main;

   int unsigned foo, bar = 10;
   int signed   foos, bars = 10;

   int unsigned wire_sum;
   int		wire_sums;

   assign wire_sum = foo + bar;
   assign wire_sums = foos + bars;

   function int unsigned sum(input int unsigned a, b);
      sum = a + b;
   endfunction

   function int unsigned sums(input int signed a, b);
      sums = a + b;
   endfunction

   initial begin
      foo = 9;
      $display("%0d * %0d = %0d", foo, bar, foo * bar);
      $display("sum(%0d,%0d) = %0d", foo, bar, sum(foo,bar));

      if (foo !== 9 || bar !== 10) begin
	 $display("FAILED");
	 $finish;
      end

      if (foo*bar !== 90) begin
	 $display("FAILED");
	 $finish;
      end

      if (sum(foo,bar) !== 19) begin
	 $display("FAILED");
	 $finish;
      end

      foos = -7;
      $display("%0d * %0d = %0d", foos, bars, foos * bars);
      $display("sums(%0d,%0d) = %0d", foos, bars, sums(foos,bars));

      if (foos !== -7 || bars !== 10) begin
	 $display("FAILED");
	 $finish;
      end

      if (foos*bars !== -70) begin
	 $display("FAILED");
	 $finish;
      end

      if (sums(foos,bars) !== 3) begin
	 $display("FAILED");
	 $finish;
      end

      #0; // allow CAs to propagate
      $display("wire_sum = %0d", wire_sum);
      $display("wire_sums = %0d", wire_sums);

      if (wire_sum !== 19) begin
	 $display("FAILED");
	 $finish;
      end

      if (wire_sums !== 3) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
