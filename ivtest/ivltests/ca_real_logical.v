module top;
  parameter parg0 = 0.0;
  parameter parg1 = 1.0;
  parameter parg2 = 2.0;
  parameter pargi = 1.0/0.0;     // Inf.
  parameter pargn = $sqrt(-1.0); // NaN.
  real arg0, arg1, arg2, argi, argn;
  reg pass;

  wire r_p0_b = !parg0;
  wire r_p1_b = !parg1;
  wire r_p2_b = !parg2;
  wire r_pi_b = !pargi;
  wire r_pn_b = !pargn;

  wire r_0_b = !arg0;
  wire r_1_b = !arg1;
  wire r_2_b = !arg2;
  wire r_i_b = !argi;
  wire r_n_b = !argn;

  wire r_p01_a = parg0 && parg1;
  wire r_p02_a = parg0 && parg2;
  wire r_p12_a = parg1 && parg2;

  wire r_01_a = arg0 && arg1;
  wire r_02_a = arg0 && arg2;
  wire r_12_a = arg1 && arg2;

  wire r_p00_o = parg0 || 0;
  wire r_p01_o = parg0 || parg1;
  wire r_p02_o = parg0 || parg2;

  wire r_00_o = arg0 || 0;
  wire r_01_o = arg0 || arg1;
  wire r_02_o = arg0 || arg2;

  wire r_p0_t = parg0 ? 1'b1 : 1'b0;
  wire r_p1_t = parg1 ? 1'b1 : 1'b0;
  wire r_p2_t = parg2 ? 1'b1 : 1'b0;
  wire r_pi_t = pargi ? 1'b1 : 1'b0;
  wire r_pn_t = pargn ? 1'b1 : 1'b0;

  wire r_0_t = arg0 ? 1'b1 : 1'b0;
  wire r_1_t = arg1 ? 1'b1 : 1'b0;
  wire r_2_t = arg2 ? 1'b1 : 1'b0;
  wire r_i_t = argi ? 1'b1 : 1'b0;
  wire r_n_t = argn ? 1'b1 : 1'b0;

  initial begin
    pass = 1'b1;

    arg0 = 0.0;
    arg1 = 1.0;
    arg2 = 2.0;
    argi = 1.0/0.0;      // Inf.
    argn = $sqrt(-1.0);  // NaN.

    #1;

    /* Check ! on a constant real value. */
    if (r_p0_b !== 1'b1) begin
      $display("Failed: CA constant !0.0, expected 1'b1, got %b", r_p0_b);
      pass = 1'b0;
    end

    if (r_p1_b !== 1'b0) begin
      $display("Failed: CA constant !1.0, expected 1'b0, got %b", r_p1_b);
      pass = 1'b0;
    end

    if (r_p2_b !== 1'b0) begin
      $display("Failed: CA constant !2.0, expected 1'b0, got %b", r_p2_b);
      pass = 1'b0;
    end

    if (r_pi_b !== 1'b0) begin
      $display("Failed: CA constant !Inf, expected 1'b0, got %b", r_pi_b);
      pass = 1'b0;
    end

    if (r_pn_b !== 1'b0) begin
      $display("Failed: CA constant !NaN, expected 1'b0, got %b", r_pn_b);
      pass = 1'b0;
    end

    /* Check ! on a real variable. */
    if (r_0_b !== 1'b1) begin
      $display("Failed: !0.0, expected 1'b1, got %b", r_0_b);
      pass = 1'b0;
    end

    if (r_1_b !== 1'b0) begin
      $display("Failed: !1.0, expected 1'b0, got %b", r_1_b);
      pass = 1'b0;
    end

    if (r_2_b !== 1'b0) begin
      $display("Failed: !2.0, expected 1'b0, got %b", r_2_b);
      pass = 1'b0;
    end

    if (r_i_b !== 1'b0) begin
      $display("Failed: !Inf, expected 1'b0, got %b", r_i_b);
      pass = 1'b0;
    end

    if (r_n_b !== 1'b0) begin
      $display("Failed: !NaN, expected 1'b0, got %b", r_n_b);
      pass = 1'b0;
    end

    /* Check && on a constant real value. */
    if (r_p01_a !== 1'b0) begin
      $display("Failed: constant 0.0 && 1.0, expected 1'b0, got %b", r_p01_a);
      pass = 1'b0;
    end

    if (r_p02_a !== 1'b0) begin
      $display("Failed: constant 0.0 && 2.0, expected 1'b0, got %b", r_p02_a);
      pass = 1'b0;
    end

    if (r_p12_a !== 1'b1) begin
      $display("Failed: constant 1.0 && 2.0, expected 1'b1, got %b", r_p12_a);
      pass = 1'b0;
    end

    /* Check && on a real variable. */
    if (r_01_a !== 1'b0) begin
      $display("Failed: 0.0 && 1.0, expected 1'b0, got %b", r_01_a);
      pass = 1'b0;
    end

    if (r_02_a !== 1'b0) begin
      $display("Failed: 0.0 && 2.0, expected 1'b0, got %b", r_02_a);
      pass = 1'b0;
    end

    if (r_12_a !== 1'b1) begin
      $display("Failed: 1.0 && 2.0, expected 1'b1, got %b", r_12_a);
      pass = 1'b0;
    end

    /* Check || on a constant real value. */
    if (r_p00_o !== 1'b0) begin
      $display("Failed: constant 0.0 || 0, expected 1'b0, got %b", r_p00_o);
      pass = 1'b0;
    end

    if (r_p01_o !== 1'b1) begin
      $display("Failed: constant 0.0 || 1.0, expected 1'b1, got %b", r_p01_o);
      pass = 1'b0;
    end

    if (r_p02_o !== 1'b1) begin
      $display("Failed: constant 0.0 || 2.0, expected 1'b1, got %b", r_p02_o);
      pass = 1'b0;
    end

    /* Check || on a real variable. */
    if (r_00_o !== 1'b0) begin
      $display("Failed: 0.0 || 0, expected 1'b0, got %b", r_00_o);
      pass = 1'b0;
    end

    if (r_01_o !== 1'b1) begin
      $display("Failed: 0.0 || 1.0, expected 1'b1, got %b", r_01_o);
      pass = 1'b0;
    end

    if (r_02_o !== 1'b1) begin
      $display("Failed: 0.0 || 2.0, expected 1'b1, got %b", r_02_o);
      pass = 1'b0;
    end

    /* Check the ternary with a constant real cond. value. */
    if (r_p0_t !== 1'b0) begin
      $display("Failed: constant 0.0 ? ..., expected 1'b0, got %b", r_p0_t);
      pass = 1'b0;
    end

    if (r_p1_t !== 1'b1) begin
      $display("Failed: constant 1.0 ? ..., expected 1'b1, got %b", r_p1_t);
      pass = 1'b0;
    end

    if (r_p2_t !== 1'b1) begin
      $display("Failed: constant 2.0 ? ..., expected 1'b1, got %b", r_p2_t);
      pass = 1'b0;
    end

    if (r_pi_t !== 1'b1) begin
      $display("Failed: constant Inf ? ..., expected 1'b1, got %b", r_pi_t);
      pass = 1'b0;
    end

    if (r_pn_t !== 1'b1) begin
      $display("Failed: constant NaN ? ..., expected 1'b1, got %b", r_pn_t);
      pass = 1'b0;
    end

    /* Check the ternary with a real cond. variable. */
    if (r_0_t !== 1'b0) begin
      $display("Failed: 0.0 ? ..., expected 1'b0, got %b", r_0_t);
      pass = 1'b0;
    end

    if (r_1_t !== 1'b1) begin
      $display("Failed: 1.0 ? ..., expected 1'b1, got %b", r_1_t);
      pass = 1'b0;
    end

    if (r_2_t !== 1'b1) begin
      $display("Failed: 2.0 ? ..., expected 1'b1, got %b", r_2_t);
      pass = 1'b0;
    end

    if (r_i_t !== 1'b1) begin
      $display("Failed: Inf ? ..., expected 1'b1, got %b", r_i_t);
      pass = 1'b0;
    end

    if (r_n_t !== 1'b1) begin
      $display("Failed: NaN ? ..., expected 1'b1, got %b", r_n_t);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
