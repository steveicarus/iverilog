// Check that event controls reject a part select on a scalar named event.

module test;

  event e;

  initial begin
    @(e[1:0]); // Error: A scalar event cannot have a part select.
    $display("FAILED");
  end

endmodule
