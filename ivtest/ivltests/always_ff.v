module top;
  reg q, clk, d;
  reg pass;

  always_ff @(posedge clk) begin
    q <= d;
  end

  initial begin
    pass = 1'b1;

    #1;
    if (q !== 1'bx) begin
      $display("FAILED: initally expected 1'bx, got %b", q);
      pass = 1'b0;
    end

    d = 1'b0;
    clk = 1'b1;
    #1;
    if (q !== 1'b0) begin
      $display("FAILED: clock in a 0 expected 1'b0, got %b", q);
      pass = 1'b0;
    end

    d = 1'b1;
    clk = 1'b0;
    #1;
    if (q !== 1'b0) begin
      $display("FAILED: no clock change expected 1'b0, got %b", q);
      pass = 1'b0;
    end

    clk = 1'b1;
    #1;
    if (q !== 1'b1) begin
      $display("FAILED: clock in a 1 expected 1'b1, got %b", q);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
