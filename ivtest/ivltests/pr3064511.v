module top;
  parameter param = -1;

  reg passed;
  wire [3:0] val = 11;
  wire [3:0] res = val + param;

  reg [3:0] rgval = 11;
  reg [3:0] rgres;

  initial begin
    passed = 1'b1;
    #1;

    if (res !== 10) begin
      $display("FAILED wire result, expected 10, got %d", res);
      passed = 1'b0;
    end

    rgres = rgval + param;
    if (rgres !== 10) begin
      $display("FAILED reg result, expected 10, got %d", rgres);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
