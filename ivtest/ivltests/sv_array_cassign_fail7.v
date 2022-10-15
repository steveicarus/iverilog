// Check that it is an error trying to continuously assign a scalar variable to
// an unpacked array.

module test;

  wire [1:0] x[1:0];
  reg [1:0] y;

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
