module top;
  reg pass = 1'b1;

  parameter one = 1'b1;
  parameter zero = 1'b0;
  parameter udef = 1'bx;

  real rl1 = one  ? 4 : 4.5; // 4.0
  real rl2 = zero ? 4.0 : 5; // 5.0
  real rl3 = udef ? 6 : 6.0; // 6.0
  real rl4 = udef ? 7 : 7;   // 7.0

  initial begin
    #1;
    if (rl1 != 4.0) begin
      $display("FAILED: real expression one, expected 4.0, got %f", rl1);
      pass = 1'b0;
    end

    if (rl2 != 5.0) begin
      $display("FAILED: real expression two, expected 5.0, got %f)", rl2);
      pass = 1'b0;
    end

    if (rl3 != 6.0) begin
      $display("FAILED: real expression three, expected 6.0, got %f", rl3);
      pass = 1'b0;
    end

    if (rl4 != 7.0) begin
      $display("FAILED: real expression four, expected 7.0, got %f", rl4);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
