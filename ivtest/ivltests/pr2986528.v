// Icarus has a number of places where it can calculate %.
module top;
  parameter out0 = 64'shF333333333333392 % 3'sd3;
  reg passed;
  wire signed  [63:0] in;
  wire signed  [2:0] const_w0;
  reg signed  [63:0] out1;
  wire signed  [63:0] out2;
  reg signed  [63:0] out3;

  assign in = 64'hF333333333333392;
  assign const_w0 = 3'sd3;

  always @* begin
    out1 = (in % const_w0);
  end

  assign out2 = (in % const_w0);

  initial begin
    passed = 1'b1;
    #1;
    $display("Testing %0d %% %0d.", in, const_w0);
    // Check the parameter result.
    if (out0 !== -2) begin
      $display("Failed: constant %%, expected -2, got %0d.", out0);
      passed = 1'b0;
    end
    // Check the always result.
    if (out1 !== -2) begin
      $display("Failed: procedural %%, expected -2, got %0d.", out1);
      passed = 1'b0;
    end
    // Check the CA result.
    if (out2 !== -2) begin
      $display("Failed: CA %%, expected -2, got %0d.", out2);
      passed = 1'b0;
    end
    // Check a compile time constant result.
    out3 = 64'shF333333333333392 % 3'sd3;
    if (out3 !== -2) begin
      $display("Failed: CA %%, expected -2, got %0d.", out3);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
    $finish;
  end
endmodule
