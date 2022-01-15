// pr1701921

module top;

   reg foo, bar;
   wire blend;

   assign blend = foo;
   assign blend = bar;

   initial begin
      bar = 1;
      // Bar explicitly has a 1 value, foo gets its initial x value.
      // Together, they should drive to an x value.
      #1 if (blend !== 1'bx) begin
	 $display("FAILED -- blend=%b (foo=%b, bar=%b)", blend, foo, bar);
	 $finish;
      end

      $display("PASSED");
   end

endmodule
