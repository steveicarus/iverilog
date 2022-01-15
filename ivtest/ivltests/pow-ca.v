module top;
  reg signed [7:0] neg = -2;
  reg signed [7:0] m1 = -1;
  reg signed [7:0] zero = 0;
  reg signed [7:0] one = 1;
  reg signed [7:0] pos = 2;
  reg signed [7:0] pose = 2;
  reg signed [7:0] poso = 3;
  reg signed [7:0] res;

  wire signed [7:0] neg_pose = neg**pose;
  wire signed [7:0] neg_poso = neg**poso;
  wire signed [7:0] m1_pose = m1**pose;
  wire signed [7:0] m1_poso = m1**poso;
  wire signed [7:0] zero_pos = zero**pos;
  wire signed [7:0] one_pos = one**pos;
  wire signed [7:0] pos_pos = pos**pos;

  wire signed [7:0] neg_zero = neg**zero;
  wire signed [7:0] m1_zero = m1**zero;
  wire signed [7:0] zero_zero = zero**zero;
  wire signed [7:0] one_zero = one**zero;
  wire signed [7:0] pos_zero = pos**zero;

  wire signed [7:0] neg_neg = neg**m1;
  wire signed [7:0] m1_nege = m1**neg;
  wire signed [7:0] m1_nego = m1**m1;
  wire signed [7:0] zero_neg = zero**m1;
  wire signed [7:0] one_neg = one**m1;
  wire signed [7:0] pos_neg = pos**m1;

  reg pass;

  initial begin
    pass = 1'b1;

    #1;

    /* Positive exponent. */
    if (neg_pose !== 4) begin
      $display("Failed neg**pos even, got %d", neg_pose);
      pass = 1'b0;
    end
    if (neg_poso !== -8) begin
      $display("Failed neg**pos odd, got %d", neg_poso);
      pass = 1'b0;
    end
    if (m1_pose !== 1) begin
      $display("Failed -1**pos even, got %d", m1_pose);
      pass = 1'b0;
    end
    if (m1_poso !== -1) begin
      $display("Failed -1**pos odd, got %d", m1_poso);
      pass = 1'b0;
    end
    if (zero_pos !== 0) begin
      $display("Failed 0**pos, got %d", zero_pos);
      pass = 1'b0;
    end
    if (one_pos !== 1) begin
      $display("Failed 1**pos, got %d", one_pos);
      pass = 1'b0;
    end
    if (pos_pos !== 4) begin
      $display("Failed 1**pos, got %d", pos_pos);
      pass = 1'b0;
    end

    /* Zero exponent. */
    if (neg_zero !== 1) begin
      $display("Failed neg**0, got %d", neg_zero);
      pass = 1'b0;
    end
    if (m1_zero !== 1) begin
      $display("Failed -1**0, got %d", m1_zero);
      pass = 1'b0;
    end
    if (zero_zero !== 1) begin
      $display("Failed 0**0, got %d", zero_zero);
      pass = 1'b0;
    end
    if (one_zero !== 1) begin
      $display("Failed 1**0, got %d", one_zero);
      pass = 1'b0;
    end
    if (pos_zero !== 1) begin
      $display("Failed pos**0, got %d", pos_zero);
      pass = 1'b0;
    end

    /* Negative exponent. */
    if (neg_neg !== 0) begin
      $display("Failed neg**neg got %d", neg_neg);
      pass = 1'b0;
    end
    if (m1_nege !== 1) begin
      $display("Failed -1**neg (even) got %d", m1_nege);
      pass = 1'b0;
    end
    if (m1_nego !== -1) begin
      $display("Failed -1**neg (odd) got %d", m1_nego);
      pass = 1'b0;
    end
    if (zero_neg !== 'sbx) begin
      $display("Failed 0**neg (odd) got %d", zero_neg);
      pass = 1'b0;
    end
    if (one_neg !== 1) begin
      $display("Failed 1**neg got %d", one_neg);
      pass = 1'b0;
    end
    if (pos_neg !== 0) begin
      $display("Failed pos**neg got %d", pos_neg);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
