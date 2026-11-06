// Check that an unbound identifier in an unpacked array assignment pattern
// results in an error, both for a procedural assignment and for a variable
// initializer.

module test;

  int ua[2];
  int ub[2] = '{1, nope}; // Should fail, nope is not declared.

  initial begin
    ua = '{undefined_id, 1}; // Should fail, undefined_id is not declared.
  end

endmodule
