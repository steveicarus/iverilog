// Check that event controls reject a bit select on a scalar named event.

module test;

  event e;

  initial begin
    @(e[0]); // Error: A scalar event cannot have a bit select.
    $display("FAILED");
  end

endmodule
