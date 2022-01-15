module top;
  parameter neg_pose = -2**2;
  parameter neg_poso = -2**3;
  parameter m1_pose = -1**2;
  parameter m1_poso = -1**3;
  parameter zero_pos = 0**2;
  parameter one_pos = 1**2;
  parameter pos_pos = 2**2;

  parameter neg_zero = -2**0;
  parameter m1_zero = -1**0;
  parameter zero_zero = 0**0;
  parameter one_zero = 1**0;
  parameter pos_zero = 2**0;

  parameter neg_neg = -2**-1;
  parameter m1_nege = -1**-2;
  parameter m1_nego = -1**-1;
  parameter zero_neg = 0**-1;
  parameter one_neg = 1**-1;
  parameter pos_neg = 2**-1;

  reg pass;

  initial begin
    pass = 1'b1;

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
