/*
 * This tests the handling of signed/types parameters. This is a test
 * of the complaints from pr 1449749.
 */
module main;

   parameter foo = -2;
   parameter integer bar = -3;
   parameter signed [5:0] bat = -7;

   initial begin
      $display("foo=%d, bar=%d, tmp=%d", foo, bar, bat);

      if (foo >= 0) begin
	 $display("FAILED -- -2 > 0");
	 $finish;
      end

      if (foo != -2) begin
	 $display("FAILED");
	 $finish;
      end

      if (bar >= 0) begin
	 $display("FAILED -- -3 > 0");
	 $finish;
      end

      if (bar != -3) begin
	 $display("FAILED");
	 $finish;
      end

      if (bat >= 0) begin
	 $display("FAILED -- -7 > 0");
	 $finish;
      end

      if (bat != -7) begin
	 $display("FAILED");
	 $finish;
      end

      $display("PASSED");
   end
endmodule // main
