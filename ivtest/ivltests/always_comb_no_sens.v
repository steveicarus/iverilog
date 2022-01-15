module test;
   reg passed;
   logic y;

   always_comb begin
      y = 1'b0;
   end

  initial begin
    passed = 1'b1;
    if (y !== 1'bx) begin
      $display("FAILED: expected 1'bx, got %b", y);
      passed = 1'b0;
    end
    #1;
    if (y !== 1'b0) begin
      $display("FAILED: expected 1'b0, got %b", y);
      passed = 1'b0;
    end
    if (passed) $display("PASSED");
  end
endmodule
