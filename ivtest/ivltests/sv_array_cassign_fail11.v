// Check that it is an error to continuously assign an unpacked array with an
// enum element type if the other unpacked array element type is not the same
// enum type, even if the two element types are the same size.

module test;

  wire enum integer {
    A
  } x[1:0];
  integer y[1:0];

  assign x = y;

  initial begin
    $display("FAILED");
  end

endmodule
