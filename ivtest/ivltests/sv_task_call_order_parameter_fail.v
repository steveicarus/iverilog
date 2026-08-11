// Check that a later parameter in the local scope hides a compilation-unit
// task.

task value;
endtask

module test;

  initial value();

  parameter value = 1;

endmodule
