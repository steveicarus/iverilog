module top;
  reg a, enb, q;
  reg pass;

  always_latch if (enb !== 1'b1) q <= a !== 1'bx;


  initial begin
    pass = 1'b1;
    #1;
    if (q !== 1'b0) begin
      $display("Expected q = 1'b0 with the default 1'bx input, got %b", q);
      pass = 1'b0;
    end

    a = 1'b0;
    #1;
    if (q !== 1'b1) begin
      $display("Expected q = 1'b1 with an explicit 1'b0 input, got %b", q);
      pass = 1'b0;
    end

    a = 1'b1;
    #1;
    if (q !== 1'b1) begin
      $display("Expected q = 1'b1 with an explicit 1'b1 input, got %b", q);
      pass = 1'b0;
    end

    a = 1'bz;
    #1;
    if (q !== 1'b1) begin
      $display("Expected q = 1'b1 with an explicit 1'bz input, got %b", q);
      pass = 1'b0;
    end

    a = 1'bx;
    #1;
    if (q !== 1'b0) begin
      $display("Expected q = 1'b0 with an explicit 1'bx input, got %b", q);
      pass = 1'b0;
    end

    enb = 1'b1;
    a = 1'bz;
    #1;
    if (q !== 1'b0) begin
      $display("Expected q = 1'b0 with an enb = 1'b1, got %b", q);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

endmodule
