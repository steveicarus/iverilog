// pr1946411

module test();
   localparam integer a = 99;

   initial begin
      if (a !== 99) begin
	 $display("FAILED -- a = %d", a);
	 $finish;
      end
      $display("PASSED\n");
   end

endmodule
