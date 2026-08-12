// Check that a local named event hides a compilation-unit task used as a
// disable target.

task value;
endtask

module test;

  event value;

  initial disable value;

endmodule
