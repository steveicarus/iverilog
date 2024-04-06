// Test we detect and report circular dependencies.

// Strictly speaking this is not legal as it uses a hierarchical name in a
// constant expression,

module test;

reg [$bits(test.b):1] a;
reg [$bits(test.a):1] b;

initial begin
  // This test is expected to fail at compile time.
  $display("FAILED");
end

endmodule
