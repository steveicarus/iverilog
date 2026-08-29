// Check that a local parameter hides a compilation-unit task used as a
// disable target.

task value;
endtask

module test;

  parameter value = 1;

  initial disable value;

endmodule
