module test;
  reg pass = 1'b1;

  parameter con = 1 * -2;
  parameter a = 1;
  parameter b = -2;
  parameter mul = a * b;
  parameter sum = a + b;
  parameter div = b / a;
  parameter sub = b - a;

  initial begin
    if (con != -2) begin
      $display("FAILED: constant mult. expected -2, got %d (%b)", con, con);
      pass = 1'b0;
    end

    if (mul != -2) begin
      $display("FAILED: multiplication expected -2, got %d (%b)", mul, mul);
      pass = 1'b0;
    end

    if (div != -2) begin
      $display("FAILED: division expected -2, got %d (%b)", div, div);
      pass = 1'b0;
    end

    if (sum != -1) begin
      $display("FAILED: summation expected -1, got %d (%b)", sum, sum);
      pass = 1'b0;
    end

    if (sub != -3) begin
      $display("FAILED: subtraction expected -3, got %d (%b)", sub, sub);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
