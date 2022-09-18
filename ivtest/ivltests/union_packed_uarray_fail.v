// Check that declaring an unpacked array typed member in a packed union is an
// error.

module test;

  struct packed {
    int x;
    shortint y[2];
  } s;

  initial begin
    $display("FAILED");
  end

endmodule
