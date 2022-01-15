module top;
  reg pass;
  reg result;
  reg [3:0] expr;

  initial begin
    pass = 1'b1;

    result = $isunknown(1'b0);
    if (result != 0) begin
      $display("FAILED: for 1'b0 expected 0, got %b", result);
      pass = 1'b0;
    end

    result = $isunknown(1'b1);
    if (result != 0) begin
      $display("FAILED: for 1'b1 expected 0, got %b", result);
      pass = 1'b0;
    end

    result = $isunknown(2'b01);
    if (result != 0) begin
      $display("FAILED: for 2'b01 expected 0, got %b", result);
      pass = 1'b0;
    end

    result = $isunknown(4'b0x11);
    if (result != 1) begin
      $display("FAILED: for 4'b0x11 expected 1, got %b", result);
      pass = 1'b0;
    end

    expr = 4'b110x;
    result = $isunknown(expr);
    if (result != 1) begin
      $display("FAILED: for 4'b110x expected 1, got %b", result);
      pass = 1'b0;
    end

    result = $isunknown(34'bx100000000000000000000000000000001);
    if (result != 1) begin
      $display("FAILED: for 34'x100000000000000000000000000000001 expected 1, got %b", result);
      pass = 1'b0;
    end

    result = $isunknown(34'b100000000000000000000000000000000x);
    if (result != 1) begin
      $display("FAILED: for 34'100000000000000000000000000000000x expected 1, got %b", result);
      pass = 1'b0;
    end

    result = $isunknown(34'b1000000000000000000000000000000000);
    if (result != 0) begin
      $display("FAILED: for 34'1000000000000000000000000000000000 expected 0, got %b", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
