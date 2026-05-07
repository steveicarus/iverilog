// Check that string literals cannot be assigned directly to multi-dimensional
// byte arrays.

module test;

  byte value [0:1][0:3] = "AB"; // Error: string is not nested

endmodule
