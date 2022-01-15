module top;
  tb #(1024) dut();
  defparam dut.Y = 2048;
endmodule

module tb;
  reg pass = 1'b1;
  parameter Z = 256;
  parameter Y = 128;
  parameter B = $clog2(Z);
  localparam C = $clog2(Y);

  initial begin
    if (B !== 10) begin
      $display("FAILED: parameter value, expected 10, got %0d", B);
      pass = 1'b0;
    end
    if (C !== 11) begin
      $display("FAILED: localparam value, expected 11, got %0d", C);
      pass = 1'b0;
    end
    if (pass) $display("PASSED");
  end
endmodule
