module top;
  reg pass;
  integer result;
  reg [3:0] expr;

  initial begin
    pass = 1'b1;

    result = $countones(1'b0);
    if (result != 0) begin
      $display("FAILED: for 1'b0 expected a count of 0, got %d", result);
      pass = 1'b0;
    end

    result = $countones(1'b1);
    if (result != 1) begin
      $display("FAILED: for 1'b1 expected a count of 1, got %d", result);
      pass = 1'b0;
    end

    result = $countones(2'b01);
    if (result != 1) begin
      $display("FAILED: for 2'b01 expected a count of 1, got %d", result);
      pass = 1'b0;
    end

    result = $countones(4'b0111);
    if (result != 3) begin
      $display("FAILED: for 4'b0111 expected a count of 3, got %d", result);
      pass = 1'b0;
    end

    expr = 4'b1100;
    result = $countones(expr);
    if (result != 2) begin
      $display("FAILED: for 4'b1100 expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    result = $countones(34'b1100000000000000000000000000000001);
    if (result != 3) begin
      $display("FAILED: for 34'1100000000000000000000000000000001 expected a count of 3, got %d", result);
      pass = 1'b0;
    end

    result = $countones(34'b1111000000110000001100000011000000);
    if (result != 10) begin
      $display("FAILED: for 34'1111000000110000001100000011000000 expected a count of 10, got %d", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
