// Check that a local variable hides a compilation-unit task used as a
// disable target.

task value;
endtask

module test;

  integer value;

  initial disable value;

endmodule
