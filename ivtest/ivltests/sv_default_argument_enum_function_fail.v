// Check that integer defaults for enum function arguments require a cast.

module test;

  typedef enum logic [3:0] {A = 1, B = 2} value_t;

  function value_t f(input value_t value = 4'd1); // Error: An enum cast is required.
    return value;
  endfunction

  initial $display("%0d", f());

endmodule
