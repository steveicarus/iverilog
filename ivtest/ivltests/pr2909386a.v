// Check the power operator (compile time).
module top;
  reg pass;

  integer res;

  initial begin
    pass = 1'b1;

    // Check the constant ** with various arguments (table 5-6 1364-2005).

    res = -3**'bx;
    if (res !== 'bx) begin
      $display("Failed: constant -3**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = -1**'bx;
    if (res !== 'bx) begin
      $display("Failed: constant -1**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 0**'bx;
    if (res !== 'bx) begin
      $display("Failed: constant 0**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 1**'bx;
    if (res !== 'bx) begin
      $display("Failed: constant 1**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 3**'bx;
    if (res !== 'bx) begin
      $display("Failed: constant 3**'bx, expected 'bx, got %0d", res);
      pass = 1'b0;
    end

    res = 'bx**-3;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**-3, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 'bx**-2;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**-2, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 'bx**-1;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**-1, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 'bx**0;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**0, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 'bx**1;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**1, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 'bx**2;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**2, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 'bx**3;
    if (res !== 'bx) begin
      $display("Failed: constant 'bx**3, expected 'bx, got %0d", res);
      pass = 1'b0;
    end

    // Check the 1st line (rvalue is positive).
    res = -3**3;
    if (res !== -27) begin
      $display("Failed: constant -3**3, expected -27, got %0d", res);
      pass = 1'b0;
    end
    res = -3**2;
    if (res !== 9) begin
      $display("Failed: constant -3**2, expected 9, got %0d", res);
      pass = 1'b0;
    end

    res = -1**3;
    if (res !== -1) begin
      $display("Failed: constant -1**3, expected -1, got %0d", res);
      pass = 1'b0;
    end
    res = -1**2;
    if (res !== 1) begin
      $display("Failed: constant -1**2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 0**3;
    if (res !== 0) begin
      $display("Failed: constant 0**3, expected 0, got %0d", res);
      pass = 1'b0;
    end
    res = 0**2;
    if (res !== 0) begin
      $display("Failed: constant 0**2, expected 0, got %0d", res);
      pass = 1'b0;
    end

    res = 1**3;
    if (res !== 1) begin
      $display("Failed: constant 1**3, expected 1, got %0d", res);
      pass = 1'b0;
    end
    res = 1**2;
    if (res !== 1) begin
      $display("Failed: constant 1**2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 3**3;
    if (res !== 27) begin
      $display("Failed: constant 3**3, expected 27, got %0d", res);
      pass = 1'b0;
    end
    res = 3**2;
    if (res !== 9) begin
      $display("Failed: constant 3**2, expected 9, got %0d", res);
      pass = 1'b0;
    end

    // Check the 2nd line (rvalue is zero).
    res = -3**0;
    if (res !== 1) begin
      $display("Failed: constant -3**0, expected 1, got %0d", res);
      pass = 1'b0;
    end
    res = -2**0;
    if (res !== 1) begin
      $display("Failed: constant -2**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = -1**0;
    if (res !== 1) begin
      $display("Failed: constant -1**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 0**0;
    if (res !== 1) begin
      $display("Failed: constant 0**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 1**0;
    if (res !== 1) begin
      $display("Failed: constant 1**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 2**0;
    if (res !== 1) begin
      $display("Failed: constant 2**0, expected 1, got %0d", res);
      pass = 1'b0;
    end
    res = 3**0;
    if (res !== 1) begin
      $display("Failed: constant 3**0, expected 1, got %0d", res);
      pass = 1'b0;
    end

    // Check the 3rd line (rvalue is negative).
    res = -2**-3;
    if (res !== 0) begin
      $display("Failed: constant -2**-3, expected 0, got %0d", res);
      pass = 1'b0;
    end
    res = -2**-2;
    if (res !== 0) begin
      $display("Failed: constant -2**-2, expected 0, got %0d", res);
      pass = 1'b0;
    end

    res = -1**-3;
    if (res !== -1) begin
      $display("Failed: constant -1**-3, expected -1, got %0d", res);
      pass = 1'b0;
    end
    res = -1**-2;
    if (res !== 1) begin
      $display("Failed: constant -1**-2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 0**-3;
    if (res !== 'bx) begin
      $display("Failed: constant 0**-3, expected 'bx, got %0d", res);
      pass = 1'b0;
    end
    res = 0**-2;
    if (res !== 'bx) begin
      $display("Failed: constant 0**-2, expected 'bx, got %0d", res);
      pass = 1'b0;
    end

    res = 1**-3;
    if (res !== 1) begin
      $display("Failed: constant 1**-3, expected 1, got %0d", res);
      pass = 1'b0;
    end
    res = 1**-2;
    if (res !== 1) begin
      $display("Failed: constant 1**-2, expected 1, got %0d", res);
      pass = 1'b0;
    end

    res = 2**-3;
    if (res !== 0) begin
      $display("Failed: constant 2**-3, expected 0, got %0d", res);
      pass = 1'b0;
    end
    res = 2**-2;
    if (res !== 0) begin
      $display("Failed: constant 2**-2, expected 0, got %0d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
