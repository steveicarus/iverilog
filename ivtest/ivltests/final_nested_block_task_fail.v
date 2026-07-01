// Check that final context is preserved when elaborating nested blocks.

module test;

  task t;
  endtask

  final begin : nested
    t;
  end

endmodule
