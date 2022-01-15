module top;
  reg pass = 1'b1;
  reg signed [7:0] in;
  wire real out = in;

  initial begin
//    $monitor(in,, out);
    #1;
    if (out != 0.0) begin
      $display("Failed: initial value, expected 0.0, got %g", out);
      pass = 1'b0;
    end

    in = 0;
    #1;
    if (out != 0.0) begin
      $display("Failed: 0 value, expected 0.0, got %g", out);
      pass = 1'b0;
    end

    in = 1;
    #1;
    if (out != 1.0) begin
      $display("Failed: 1 value, expected 1.0, got %g", out);
      pass = 1'b0;
    end

    in = -1;
    #1;
    if (out != -1.0) begin
      $display("Failed: -1 value, expected -1.0, got %g", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
