// Check that an unpacked array assignment pattern with too many elements
// results in an error.

module test;

  int x[1:0];

  initial begin
    x = '{1, 2, 3}; // Should fail, more elements in assignment pattern than
                    // array size.
  end

endmodule
