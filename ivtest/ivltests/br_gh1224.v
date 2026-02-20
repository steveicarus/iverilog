// Test for GitHub issue #1224
// Packed vs unpacked dimension confusion with byte array
module a(output byte b [0:0]);
  assign b = '{8'd1};  // Should be interpreted as single byte value
endmodule

module test;
  wire byte out_b [0:0];

  a dut(.b(out_b));

  initial begin
    #1;
    if (out_b[0] !== 8'd1) begin
      $display("FAILED: out_b[0] = %d, expected 1", out_b[0]);
      $finish;
    end
    $display("PASSED");
  end
endmodule
