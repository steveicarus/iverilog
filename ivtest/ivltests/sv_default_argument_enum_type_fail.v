// Check that enum defaults match the formal argument's enum type.

module test;

  typedef enum logic [3:0] {A = 1} first_t;
  typedef enum logic [3:0] {B = 1} second_t;

  function first_t f(input first_t value = B); // Error: The enum types differ.
    return value;
  endfunction

  initial $display("%0d", f());

endmodule
