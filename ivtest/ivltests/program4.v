// This test program shows how programs can be instantiated
// within another module.

program test(input [7:0] sh1, input [7:0] sh2);

   final begin
      if (sh1 !== 'h55) begin
	 $display("FAILED -- shared=%b is not correct", sh1);
	 $finish;
      end

      if (sh2 !== 'haa) begin
	 $display("FAILED -- sh2 not correct", sh2);
	 $finish;
      end

      $display("PASSED");
   end

endprogram :test

module main;

   reg[7:0] shared = 'h55;
   wire [7:0] not_shared = ~shared;

   test check(shared, not_shared);

endmodule // main
