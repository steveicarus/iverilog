// Check that an error is reported when using an assignment pattern on a scalar
// type.

module test;

  bit x = '{1'b1}; // This should fail. Can't use assignment pattern with
                   // scalar types

  initial begin
    $display("FAILED");
  end

endmodule
