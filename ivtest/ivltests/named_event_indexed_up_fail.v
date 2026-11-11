// Check that event controls reject an indexed part select on a scalar named event.

module test;

  event e;

  initial begin
    @(e[0+:2]); // Error: A scalar event cannot have an indexed part select.
    $display("FAILED");
  end

endmodule
