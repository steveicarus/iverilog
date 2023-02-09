// Check that an error is reported when specifing less elements in a packed
// array assignment pattern than the array size.

module test;

  bit [2:0][3:0] x = '{1, 2}; // This should fail. Less elements than array
                              // size.

  initial begin
    $display("FAILED");
  end

endmodule
