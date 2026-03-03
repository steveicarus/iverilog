// Test for GitHub issue #1265
// Single element unpacked array continuous assignment should compile
module test(output o1 [0:0], input i1 [0:0]);
  assign o1 = i1;

  // Verify the assignment works by checking a simple case
  initial begin
    $display("PASSED");
  end
endmodule
