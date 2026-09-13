// Check that an assertion label hides an outer typedef before the assertion.

typedef reg [7:0] CHECK;

module test;

  initial begin
    CHECK value; // Error: The local named block hides the outer typedef.
    CHECK: assert (1);
  end

endmodule
