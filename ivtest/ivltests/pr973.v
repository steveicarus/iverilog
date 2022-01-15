module main;

   real foo;

   initial begin
      foo = 1.0;
      if (foo != 1.0) begin
	 $display("FAILED: Simple assign works not. foo=%f", foo);
	 $finish;
      end

      foo <= 1.5;
      if (foo != 1.0) begin
	 $display("FAILED: nb assign works too fast. foo=%f", foo);
	 $finish;
      end

      #1 if (foo != 1.5) begin
	 $display("FAILED: nb assign works not. foo=%f", foo);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end // initial begin

endmodule // main
