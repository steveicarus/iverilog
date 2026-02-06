// Test for GitHub issue #1220
// Assertion failed with uwire multi-dimensional input port
module sub(input uwire data [1:0]);
  initial begin
    #1;
    if (data[0] === 1'b0 && data[1] === 1'b1)
      $display("PASSED");
    else
      $display("FAILED: data[0]=%0b data[1]=%0b", data[0], data[1]);
  end
endmodule

module test;
  wire w [1:0];
  assign w[0] = 1'b0;
  assign w[1] = 1'b1;

  sub dut(.data(w));
endmodule
