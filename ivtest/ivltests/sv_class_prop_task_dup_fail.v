// Check that a class property and task cannot have the same name.

module test;

  class C;
    task value;
    endtask

    int value;
  endclass

endmodule
