module top;
  reg pass;
  integer result;
  reg [3:0] expr;
  reg bval;

  initial begin
    pass = 1'b1;

    result = $countbits(1'bx, 1'bx);
    if (result != 1) begin
      $display("FAILED: for 1'bx/x expected a count of 1, got %d", result);
      pass = 1'b0;
    end

    result = $countbits(2'bxx, 1'bx);
    if (result != 2) begin
      $display("FAILED: for 2'bxx/x expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    result = $countbits(2'bxz, 1'bz, 1'bx);
    if (result != 2) begin
      $display("FAILED: for 2'bxz/zx expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    result = $countbits(4'b01zx, 1'bz, 1'bx);
    if (result != 2) begin
      $display("FAILED: for 4'b01zx/zx expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    result = $countbits(4'b01zx, 1'b0, 1'b1);
    if (result != 2) begin
      $display("FAILED: for 4'b01zx/01 expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    bval = 1'b0;
    expr = 4'b1001;
    result = $countbits(expr, bval);
    if (result != 2) begin
      $display("FAILED: for 4'b1001/0 expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    bval = 1'b1;
    result = $countbits(expr, bval);
    if (result != 2) begin
      $display("FAILED: for 4'b1001/1 expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    result = $countbits(34'bzx00000000000000000000000000000000, 1'bz, 1'bx);
    if (result != 2) begin
      $display("FAILED: for 34'zx00000000000000000000000000000000/zx expected a count of 2, got %d", result);
      pass = 1'b0;
    end

    result = $countbits(34'bzxxz000000xz000000xz000000xz000000, 1'bz, 1'bx);
    if (result != 10) begin
      $display("FAILED: for 34'zxxz000000xz000000xz000000xz000000/zx expected a count of 10, got %d", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
