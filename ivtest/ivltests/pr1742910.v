// pr1742910

module checktest();

   parameter sum = 1'h1 + 1'h1;

   initial begin
`ifdef __ICARUS_UNSIZED__
      if (sum !== 2) begin
`else
      if (sum !== 0) begin
`endif
	 $display("FAILED -- sum = %d", sum);
	 $finish;
      end
      $display("PASSED");
      $finish;
   end

endmodule
