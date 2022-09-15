// Check that declaring a dynamic array typed member in a packed struct is an
// error.

module test;

  struct packed {
    int x[];
  } s;

  initial begin
    $display("FAILED");
  end

endmodule
