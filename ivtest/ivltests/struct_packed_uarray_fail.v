// Check that declaring an unpacked array typed member in a packed struct is an
// error.

module test;

  struct packed {
    int x[2];
  } s;

  initial begin
    $display("FAILED");
  end

endmodule
