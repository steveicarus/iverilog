// Check that an error is reported when trying to bind the same argument by name
// multiple times.

module test;

  task t(integer a, integer b);
    $display("FAILED");
  endtask

  initial begin
    t(.a(1), .a(2));
  end

endmodule
