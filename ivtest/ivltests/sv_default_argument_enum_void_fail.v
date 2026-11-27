// Check that enum arguments of void functions reject integer defaults.

module test;

  typedef enum logic [3:0] {A = 1, B = 2} value_t;

  function void f(input value_t value = 4'd1); // Error: An enum cast is required.
    $display("%0d", value);
  endfunction

  initial f();

endmodule
