module top;
  reg pass;

  reg signed [7:0] neg = -2;
  reg signed [7:0] m1 = -1;
  reg signed [7:0] zero = 0;
  reg signed [7:0] one = 1;
  reg signed [7:0] pos = 2;
  reg signed [7:0] pose = 2;
  reg signed [7:0] poso = 3;
  reg signed [7:0] res;

  initial begin
    pass = 1'b1;

    #1;

    /* Positive exponent. */
    res = neg**pose;
    if (res !== 4) begin
      $display("Failed neg**pos even, got %d", res);
      pass = 1'b0;
    end
    res = neg**poso;
    if (res !== -8) begin
      $display("Failed neg**pos odd, got %d", res);
      pass = 1'b0;
    end
    res = m1**pose;
    if (res !== 1) begin
      $display("Failed -1**pos even, got %d", res);
      pass = 1'b0;
    end
    res = m1**poso;
    if (res !== -1) begin
      $display("Failed -1**pos odd, got %d", res);
      pass = 1'b0;
    end
    res = zero**pos;
    if (res !== 0) begin
      $display("Failed 0**pos, got %d", res);
      pass = 1'b0;
    end
    res = one**pos;
    if (res !== 1) begin
      $display("Failed 1**pos, got %d", res);
      pass = 1'b0;
    end
    res = pos**pos;
    if (res !== 4) begin
      $display("Failed 1**pos, got %d", res);
      pass = 1'b0;
    end

    /* Zero exponent. */
    res = neg**zero;
    if (res !== 1) begin
      $display("Failed neg**0, got %d", res);
      pass = 1'b0;
    end
    res = m1**zero;
    if (res !== 1) begin
      $display("Failed -1**0, got %d", res);
      pass = 1'b0;
    end
    res = zero**zero;
    if (res !== 1) begin
      $display("Failed 0**0, got %d", res);
      pass = 1'b0;
    end
    res = one**zero;
    if (res !== 1) begin
      $display("Failed 1**0, got %d", res);
      pass = 1'b0;
    end
    res = pos**zero;
    if (res !== 1) begin
      $display("Failed pos**0, got %d", res);
      pass = 1'b0;
    end

    /* Negative exponent. */
    res = neg**m1;
    if (res !== 0) begin
      $display("Failed neg**neg got %d", res);
      pass = 1'b0;
    end
    res = m1**neg;
    if (res !== 1) begin
      $display("Failed -1**neg (even) got %d", res);
      pass = 1'b0;
    end
    res = m1**m1;
    if (res !== -1) begin
      $display("Failed -1**neg (odd) got %d", res);
      pass = 1'b0;
    end
    res = zero**m1;
    if (res !== 'sbx) begin
      $display("Failed 0**neg got %d", res);
      pass = 1'b0;
    end
    res = one**m1;
    if (res !== 1) begin
      $display("Failed 1**neg got %d", res);
      pass = 1'b0;
    end
    res = pos**m1;
    if (res !== 0) begin
      $display("Failed pos**neg got %d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
