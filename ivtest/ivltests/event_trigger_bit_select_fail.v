// Check that bit selects are rejected on scalar event trigger targets.

module test;

  event e;

  initial begin
    -> e[0]; // Error: A scalar event cannot have a bit select.
    $display("FAILED");
  end

endmodule
