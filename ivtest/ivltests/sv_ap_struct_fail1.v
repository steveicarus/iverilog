// Check that it is an error to provide less elements in a struct assignment
// pattern than there are members in the struct.

module test;

  struct packed {
    int x;
    shortint y;
    byte z;
  } x = '{1, 2}; // This should fail. Less elements than required.

  initial begin
    $display("FAILED");
  end

endmodule
