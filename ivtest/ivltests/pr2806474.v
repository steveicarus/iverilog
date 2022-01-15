module top;
  reg pass;

  real a, b;
  integer i;

  wire real b1 = 42.0 + 10/100;
  wire real b2 = a + 10/100;
  wire real b3 = 42.0 + i/100;
  wire real b4 = a + i/100;

  initial begin
    pass = 1'b1;

    // Check the compiler for the whole expression.
    b = 42.0 + 10/100;
    if (b != 42.0) begin
      $display("FAILED: compiler constant, expected 42.0, got %6.1f", b);
      pass = 1'b0;
    end

    // Check the compiler for just the division.
    a = 42;
    b = a + 10/100;
    if (b != 42.0) begin
      $display("FAILED: compiler constant div., expected 42.0, got %6.1f", b);
      pass = 1'b0;
    end

    // Check the run time with a constant sum value (just the division).
    i = 10;
    b = 42.0 + i/100;
    if (b != 42.0) begin
      $display("FAILED: runtime constant real, expected 42.0, got %6.1f", b);
      pass = 1'b0;
    end

    // Check the original expression.
    b = a + i/100;
    if (b != 42.0) begin
      $display("FAILED: runtime, expected 42.0, got %6.1f", b);
      pass = 1'b0;
    end

    // Check the ternary operator with one clause needing to be converted.
    b = (i === 10) ? i/100 : 1.0;
    if (b != 0.0) begin
      $display("FAILED: runtime (ternary), expected 0.0, got %6.1f", b);
      pass = 1'b0;
    end

    b = |i;
    if (b != 1.0) begin
      $display("FAILED: runtime (reduction), expected 1.0, got %6.1f", b);
      pass = 1'b0;
    end

    // Check the continuous assigns.
    #1;
    if (b1 != 42.0) begin
      $display("FAILED: CA test 1, expected 42.0, got %6.1f", b1);
      pass = 1'b0;
    end
    if (b2 != 42.0) begin
      $display("FAILED: CA test 2, expected 42.0, got %6.1f", b2);
      pass = 1'b0;
    end
    if (b3 != 42.0) begin
      $display("FAILED: CA test 3, expected 42.0, got %6.1f", b3);
      pass = 1'b0;
    end
    if (b4 != 42.0) begin
      $display("FAILED: CA test 4, expected 42.0, got %6.1f", b4);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
