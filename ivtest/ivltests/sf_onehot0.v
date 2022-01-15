module top;
  reg pass;
  reg result;
  reg [3:0] expr;

  initial begin
    pass = 1'b1;

    result = $onehot0(1'b0);
    if (result != 1) begin
      $display("FAILED: for 1'b0 expected 1, got %b", result);
      pass = 1'b0;
    end

    result = $onehot0(1'b1);
    if (result != 1) begin
      $display("FAILED: for 1'b1 expected 1, got %b", result);
      pass = 1'b0;
    end

    result = $onehot0(2'b01);
    if (result != 1) begin
      $display("FAILED: for 2'b01 expected 1, got %b", result);
      pass = 1'b0;
    end

    result = $onehot0(4'b0x11);
    if (result != 0) begin
      $display("FAILED: for 4'b0x11 expected 0, got %b", result);
      pass = 1'b0;
    end

    expr = 4'b1100;
    result = $onehot0(expr);
    if (result != 0) begin
      $display("FAILED: for 4'b1100 expected 0, got %b", result);
      pass = 1'b0;
    end

    result = $onehot0(34'b1100000000000000000000000000000001);
    if (result != 0) begin
      $display("FAILED: for 34'1100000000000000000000000000000001 expected 0, got %b", result);
      pass = 1'b0;
    end

    result = $onehot0(34'b1000000000000000000000000000000000);
    if (result != 1) begin
      $display("FAILED: for 34'1000000000000000000000000000000000 expected 1, got %b", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
