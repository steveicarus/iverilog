// Check that it is an error trying to continuously assign a scalar expression
// to a unpacked array.

module test;

  wire [1:0] x[1:0];

  assign x = 1'b1 + 1'b1;

  initial begin
    $display("FAILED");
  end

endmodule
