// Check that the key of a typed struct pattern item must be a member name.

module test;

  typedef struct packed { logic [3:0] a; logic [3:0] b; } s_t;
  s_t s;

  initial begin
    s = s_t'{x: 4'h1, b: 4'h2};
  end

endmodule
