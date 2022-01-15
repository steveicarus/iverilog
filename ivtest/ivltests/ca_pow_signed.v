module top;
  reg passed = 1'b1;

  reg signed [15:0] a, b;

    /* Currently the result can only be as big as a native long! */
  wire signed [31:0] r = a ** b;

  initial begin

    a = 5;
    b = 2;
    #1
    if (r != 25) begin
      $display("Failed: 5 ** 2 gave %d, expected 25", r);
      passed = 1'b0;
    end

    a = -5;
    b = 3;
    #1
    if (r != -125) begin
      $display("Failed: -5 ** 3 gave %d, expected -125", r);
      passed = 1'b0;
    end

    a = 2;
    b = 30;
    #1
    if (r != 1_073_741_824) begin
      $display("Failed: 2 ** 30 gave %d, expected 1,073,741,824", r);
      passed = 1'b0;
    end

    a = -2;
    b = 31;
    #1
    if (r != -2_147_483_648) begin
      $display("Failed: -2 ** 31 gave %d, expected -2,147,483,648", r);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
