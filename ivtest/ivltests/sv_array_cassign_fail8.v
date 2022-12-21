// Check that it is an error trying to continuously assign a scalar net to an
// unpacked array.

module test;

  wire a[1:0];
  wire x;

  assign a = x;

  initial begin
    $display("FAILED");
  end

endmodule
