module top;
  reg pass;
  reg [31:0] in2;
  integer in1;
  reg signed [128:0] res;

  initial begin
    pass = 1'b1;

    in1 = -2; in2 = 63;
    res = in1 ** in2;
    if (res !== -128'sd9223372036854775808) begin
      $display("Failed: -2 ** 65, expected -9223372036854775808, got %0d", res);
      pass = 1'b0;
    end

    in1 = -2; in2 = 65;
    res = in1 ** in2;
    if (res !== -128'sd36893488147419103232) begin
      $display("Failed: -2 ** 65, expected -36893488147419103232, got %0d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
