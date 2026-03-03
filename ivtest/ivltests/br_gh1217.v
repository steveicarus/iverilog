// Test for GitHub issue #1217
// Unpacked array literal parsing
module a(output bit b [0:0]);
  assign b = '{1'b0};
endmodule

module test;
  wire bit out_b [0:0];

  a dut(.b(out_b));

  initial begin
    #1;
    if (out_b[0] !== 1'b0) begin
      $display("FAILED: out_b[0] = %b, expected 0", out_b[0]);
      $finish;
    end
    $display("PASSED");
  end
endmodule
