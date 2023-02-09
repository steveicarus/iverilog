// Check that an error is reported when specifing more elements in a packed
// array assignment pattern than the array size.

module test;

  bit [2:0][3:0] x = '{1, 2, 3, 4}; // This should fail. More elements than
                                    // array size.

  initial begin
    $display("FAILED");
  end

endmodule
