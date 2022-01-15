module top;
  reg pass;
  real rarr [1:0];
  real rat;
  integer i;
  wire real rmon = rarr[0];
  wire real rmonv = rarr[i];

  always @(rarr[0]) begin
     rat = rarr[0];
  end

  initial begin
    pass = 1'b1;
    i = 0;

    rarr[0] = 1.125;
    #1;
    if (rmon != 1.125) begin
      $display("Failed CA at 0, expected 1.125, got %6.3f", rmon);
      pass = 1'b0;
    end
    if (rmonv != 1.125) begin
      $display("Failed CA (var) at 0, expected 1.125, got %6.3f", rmonv);
      pass = 1'b0;
    end
    if (rat != 1.125) begin
      $display("Failed @ at 0, expected 1.125, got %6.3f", rat);
      pass = 1'b0;
    end

    rarr[0] = 2.25;
    #1;
    if (rmon != 2.25) begin
      $display("Failed CA at 1, expected 2.250, got %6.3f", rmon);
      pass = 1'b0;
    end
    if (rmonv != 2.25) begin
      $display("Failed CA (var) at 1, expected 2.250, got %6.3f", rmonv);
      pass = 1'b0;
    end
    if (rat != 2.25) begin
      $display("Failed @ at 1, expected 2.250, got %6.3f", rat);
      pass = 1'b0;
    end

    i = 1;
    #1
    if (rmonv != 0.0) begin
      $display("Failed CA (var) at 2, expected 0.000, got %6.3f", rmonv);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
