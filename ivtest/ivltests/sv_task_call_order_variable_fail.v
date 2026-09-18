// Check that a later local variable hides a compilation-unit task.

task value;
endtask

module test;

  initial value();

  integer value;

endmodule
