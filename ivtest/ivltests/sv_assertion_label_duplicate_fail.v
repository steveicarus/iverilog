// Check that assertion labels must be unique within their containing scope.

module test;

  initial begin
    CHECK: assert (1);
    CHECK: assert (1); // Error: Duplicate block name in the same scope.
  end

endmodule
