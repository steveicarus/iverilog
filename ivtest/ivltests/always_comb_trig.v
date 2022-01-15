module top;
  reg a, q, qb;
  reg pass;

  always_comb q = a !== 1'bx;

  always_comb qb = a === 1'bx;

  initial begin
    pass = 1'b1;
    #0;
    // This second delay is needed for vlog95 since it uses #0 to create
    // the T0 trigger. vvp also needs it since it puts the event in the
    // inactive queue just like a #0 delay.
    #0;
    if (q !== 1'b0) begin
      $display("Expected q = 1'b0 with the default 1'bx input, got %b", q);
      pass = 1'b0;
    end
    if (qb !== 1'b1) begin
      $display("Expected qb = 1'b1 with the default 1'bx input, got %b", qb);
      pass = 1'b0;
    end

    #1;
    a = 1'b0;
    #0;
    if (q !== 1'b1) begin
      $display("Expected q = 1'b1 with an explicit 1'b0 input, got %b", q);
      pass = 1'b0;
    end

    #1;
    a = 1'b1;
    #0;
    if (q !== 1'b1) begin
      $display("Expected q = 1'b1 with an explicit 1'b1 input, got %b", q);
      pass = 1'b0;
    end

    #1;
    a = 1'bz;
    #0;
    if (q !== 1'b1) begin
      $display("Expected q = 1'b1 with an explicit 1'bz input, got %b", q);
      pass = 1'b0;
    end

    #1;
    a = 1'bx;
    #0;
    if (q !== 1'b0) begin
      $display("Expected q = 1'b0 with an explicit 1'bx input, got %b", q);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
