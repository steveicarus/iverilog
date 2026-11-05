// Check that specifying a struct member twice is an error.

module test;

  struct packed { logic [3:0] a; logic [3:0] b; } s;

  initial begin
    s = '{a: 4'h1, a: 4'h2, b: 4'h3};
  end

endmodule
