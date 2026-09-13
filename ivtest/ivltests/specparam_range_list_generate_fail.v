// Check that rejecting a ranged specparam list does not crash.

module test;

  generate
    if (1) begin : g
      specparam [3:0] A = 1, B = 2; // Error: Specparams are not allowed in generate blocks.
    end
  endgenerate

endmodule
