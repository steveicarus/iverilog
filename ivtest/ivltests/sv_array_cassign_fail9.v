// Check that it is an error trying to continuously assign an element of an
// unpacked array to another unpacked array as a whole.

module test;

  wire a[1:0];
  wire x[1:0];

  assign a = x[0];

  initial begin
    $display("FAILED");
  end

endmodule
