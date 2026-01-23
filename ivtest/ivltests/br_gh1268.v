// Test for GitHub issue #1268
// Variable (output logic) should be allowed to be driven by primitive gate
// Per IEEE 1800-2017 6.5: variables can be written by one port (primitive output)

module driver(output logic c, input wire d);
  not b(c, d);
endmodule

module test;
  wire d = 1'b0;
  wire c;

  driver dut(.c(c), .d(d));

  initial begin
    #1;
    if (c !== 1'b1) begin
      $display("FAILED: c = %b, expected 1", c);
      $finish;
    end
    $display("PASSED");
  end
endmodule
