// Check that a later typedef hides a compilation-unit task.

task value;
endtask

module test;

  initial value();

  typedef logic value;

endmodule
