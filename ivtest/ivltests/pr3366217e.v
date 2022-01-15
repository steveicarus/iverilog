module top;
  reg passed;
  // These should be OK
  enum {zdef1_[0:1]} zdef1;
  enum {zdef2_[1:0]} zdef2;
  enum {zdefb_[0:0]} zdef3;
  enum {zvalb_[0:0] = 1} zval1;

  initial begin
    passed = 1'b1;

    if (zdef1_0 !== 0) begin
      $display("FAILED: expected zdef1_0 to be 0, got %0d", zdef1_0);
      passed = 1'b0;
    end
    if (zdef1_1 !== 1) begin
      $display("FAILED: expected zdef1_1 to be 1, got %0d", zdef1_1);
      passed = 1'b0;
    end

    if (zdef2_1 !== 0) begin
      $display("FAILED: expected zdef2_1 to be 0, got %0d", zdef2_1);
      passed = 1'b0;
    end
    if (zdef2_0 !== 1) begin
      $display("FAILED: expected zdef2_0 to be 1, got %0d", zdef2_0);
      passed = 1'b0;
    end

    if (zdefb_0 !== 0) begin
      $display("FAILED: expected zdefb_0 to be 0, got %0d", zdefb_0);
      passed = 1'b0;
    end

    if (zvalb_0 !== 1) begin
      $display("FAILED: expected zvalb_0 to be 1, got %0d", zvalb_0);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
