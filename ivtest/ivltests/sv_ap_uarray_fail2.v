// Check that an unpacked array assignment pattern with not enough elements
// results in an error.

module test;

  int x[1:0];

  initial begin
    x = '{1}; // Should fail, less elements in assignment pattern than array
              // size.
  end

endmodule
