// Check that a duplicate typedef diagnostic locates the earlier array type.

module test;

  typedef int T[2];
  typedef int T; // Error: The array typedef already declares T.

endmodule
