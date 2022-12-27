// Check that an error is reported when specifing a default member value for a
// packed struct.

module test;
  struct packed {
    // This should fail, default member value is not allowed for packed struct
    integer x = 10;
  } s;

  initial begin
    $display("FAILED");
  end
endmodule
