module top;
  parameter parg0 = 0.0;
  parameter parg1 = 1.0;
  parameter parg2 = 2.0;
  parameter pargi = 1.0/0.0;     // Inf.
  parameter pargn = $sqrt(-1.0); // NaN.
  real arg0, arg1, arg2, argi, argn;
  reg result, pass;

  initial begin
    pass = 1'b1;

    arg0 = 0.0;
    arg1 = 1.0;
    arg2 = 2.0;
    argi = 1.0/0.0;      // Inf.
    argn = $sqrt(-1.0);  // NaN.

    /* Check ! on a constant real value. */
    result = !parg0;
    if (result !== 1'b1) begin
      $display("Failed: constant !0.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = !parg1;
    if (result !== 1'b0) begin
      $display("Failed: constant !1.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = !parg2;
    if (result !== 1'b0) begin
      $display("Failed: constant !2.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = !pargi;
    if (result !== 1'b0) begin
      $display("Failed: constant !Inf, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = !pargn;
    if (result !== 1'b0) begin
      $display("Failed: constant !NaN, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    /* Check ! on a real variable. */
    result = !arg0;
    if (result !== 1'b1) begin
      $display("Failed: !0.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = !arg1;
    if (result !== 1'b0) begin
      $display("Failed: !1.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = !arg2;
    if (result !== 1'b0) begin
      $display("Failed: !2.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = !argi;
    if (result !== 1'b0) begin
      $display("Failed: !Inf, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = !argn;
    if (result !== 1'b0) begin
      $display("Failed: !NaN, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    /* Check && on a constant real value. */
    result = parg0 && parg1;
    if (result !== 1'b0) begin
      $display("Failed: constant 0.0 && 1.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = parg0 && parg2;
    if (result !== 1'b0) begin
      $display("Failed: constant 0.0 && 2.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = parg1 && parg2;
    if (result !== 1'b1) begin
      $display("Failed: constant 1.0 && 2.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    /* Check && on a real variable. */
    result = arg0 && arg1;
    if (result !== 1'b0) begin
      $display("Failed: 0.0 && 1.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = arg0 && arg2;
    if (result !== 1'b0) begin
      $display("Failed: 0.0 && 2.0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = arg1 && arg2;
    if (result !== 1'b1) begin
      $display("Failed: 1.0 && 2.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    /* Check || on a constant real value. */
    result = parg0 || 0;
    if (result !== 1'b0) begin
      $display("Failed: constant 0.0 || 0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = parg0 || parg1;
    if (result !== 1'b1) begin
      $display("Failed: constant 0.0 || 1.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = parg0 || parg2;
    if (result !== 1'b1) begin
      $display("Failed: constant 0.0 || 2.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    /* Check || on a real variable. */
    result = arg0 || 0;
    if (result !== 1'b0) begin
      $display("Failed: 0.0 || 0, expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = arg0 || arg1;
    if (result !== 1'b1) begin
      $display("Failed: 0.0 || 1.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = arg0 || arg2;
    if (result !== 1'b1) begin
      $display("Failed: 0.0 || 2.0, expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    /* Check the ternary with a constant real cond. value. */
    result = parg0 ? 1'b1 : 1'b0;
    if (result !== 1'b0) begin
      $display("Failed: constant 0.0 ? ..., expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = parg1 ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: constant 1.0 ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = parg2 ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: constant 2.0 ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = pargi ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: constant Inf ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = pargn ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: constant NaN ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    /* Check the ternary with a real cond. variable. */
    result = arg0 ? 1'b1 : 1'b0;
    if (result !== 1'b0) begin
      $display("Failed: 0.0 ? ..., expected 1'b0, got %b", result);
      pass = 1'b0;
    end

    result = arg1 ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: 1.0 ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = arg2 ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: 2.0 ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = argi ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: Inf ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    result = argn ? 1'b1 : 1'b0;
    if (result !== 1'b1) begin
      $display("Failed: NaN ? ..., expected 1'b1, got %b", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
