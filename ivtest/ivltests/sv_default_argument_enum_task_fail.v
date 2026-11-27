// Check that integer defaults for enum task arguments require a cast.

module test;

  typedef enum logic [3:0] {A = 1, B = 2} value_t;

  task t(input value_t value = 4'd1); // Error: An enum cast is required.
    $display("%0d", value);
  endtask

  initial t();

endmodule
