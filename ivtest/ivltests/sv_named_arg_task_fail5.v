// Check that an error is reported when binding an empty value to an argument by
// name and the argument does not have a default value.

module test;

  task t(integer a);
    $display("FAILED");
  endtask

  initial begin
    t(.a()); // This should fail. `a` has no default value.
  end

endmodule
