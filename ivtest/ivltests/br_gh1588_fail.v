// Check that assigning an undefined value to an enum reports an error.

module test;
  enum { IDLE, RUN } state;

  initial begin
    state = IDEL; // Error: IDEL is not declared.
  end
endmodule
