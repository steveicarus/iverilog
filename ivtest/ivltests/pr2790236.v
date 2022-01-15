module test (a, b);
  output a;
  reg a = 1'b0;
  output reg b = 1'b1;
endmodule

module top;
  wire out1, out2;

  test dut(out1, out2);

  initial begin
    #1;
    if (out1 !== 1'b0 || out2 !== 1'b1) begin
      $display("Failed: expected 0:1, got %b:%b", out1, out2);
    end else $display("PASSED");
  end
endmodule
