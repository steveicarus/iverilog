/*
 * This demonstrates that strings can be used as
 * constructed formats in $display et al.
 */
module main;

   string foo;

   initial begin
      foo = "PAXXED";
      $display("foo.len()=%0d (s.b. 6)", foo.len());
      if (foo.len() != 6) begin
	 $display("FAILED -- foo.len() = %0d", foo.len());
	 $finish;
      end

      $display("foo[-1]=%b (s.b. 00000000)", foo[-1]);
      if (foo[-1] != 'h00) begin
	 $display("FAILED -- foo[-1]=%h", foo[-1]);
	 $finish;
      end

      $display("foo[7]=%b (s.b. 00000000)", foo[7]);
      if (foo[7] != 'h00) begin
	 $display("FAILED -- foo[7]=%h", foo[7]);
	 $finish;
      end

      $display("foo[4]=%b (s.b. 01000101)", foo[4]);
      if (foo[4] != 'h45) begin
	 $display("FAILED -- foo[4]=%h", foo[4]);
	 $finish;
      end

      foo[2] = 'h00;
      if (foo != "PAXXED") begin
	 $display("FAILED -- foo=%0s (1)", foo);
	 $finish;
      end

      foo[2]='h53;
      foo[3]='h53;

      $display(foo);
   end
endmodule // main
