// Check that a later named event in the local scope hides a compilation-unit
// task.

task value;
endtask

module test;

  initial value();

  event value;

endmodule
