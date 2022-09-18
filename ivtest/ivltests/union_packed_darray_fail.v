// Check that declaring a dynamic array typed member in a packed union is an
// error.

module test;

  union packed {
    int x[];
  } s;

  initial begin
    $display("FAILED");
  end

endmodule
