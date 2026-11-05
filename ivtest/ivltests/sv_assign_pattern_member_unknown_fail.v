// Check that an unknown struct member name in a pattern is an error.

module test;

  struct packed { logic [3:0] a; logic [3:0] b; } s;

  initial begin
    s = '{a: 4'h1, c: 4'h2};
  end

endmodule
