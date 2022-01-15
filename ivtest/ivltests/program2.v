// This program block demonstrates that initial and final blocks
// work, and that program variables work as well.
program main;

   int foo;
   int bar;

   initial begin
      bar = 1;
      for (foo = 1 ; foo < 10 ; ++foo) begin
	 bar = bar * foo;
	 $display("foo = %d, bar=%d", foo, bar);
      end
   end

   final begin
      if (foo !== 10 || bar !== 362_880) begin
	 $display("FAILED -- foo=%d", foo);
      end else begin
	 $display("PASSED");
      end
   end

endprogram // main
