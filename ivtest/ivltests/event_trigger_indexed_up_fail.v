// Check that ascending indexed part selects are rejected on event trigger targets.

module test;

  event e;

  initial begin
    -> e[0+:2]; // Error: An event trigger target cannot have a part select.
    $display("FAILED");
  end

endmodule
