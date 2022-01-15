module top;
  reg passed;

  initial begin
    passed = 1'b1;
    if ($test$plusargs("opt") == 0) begin
      $display("Failed to find +opt");
      passed = 1'b0;
    end
    if ($test$plusargs("option") == 0) begin
      $display("Failed to find +option");
      passed = 1'b0;
    end
    if ($test$plusargs("options") != 0) begin
      $display("Failed, found +options");
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
