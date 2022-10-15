// Check that it is an error trying to continuously assign an unpacked array
// with an enum element type to another unpacked array with an element type that
// is not the same enum type, even if the two element types are the same size.

module test;

  wire integer  x[1:0];
  enum integer {
    A
  } y[1:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
