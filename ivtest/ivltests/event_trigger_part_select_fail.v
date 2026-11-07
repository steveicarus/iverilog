// Check that part selects are rejected on event trigger targets.

module test;

  event e;

  initial begin
    -> e[1:0]; // Error: An event trigger target cannot have a part select.
    $display("FAILED");
  end

endmodule
