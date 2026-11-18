// Check that a typed pattern needs as many items as the type has members.

module test;

  typedef struct packed { logic [3:0] a; logic [3:0] b; } s_t;
  s_t s;

  initial begin
    s = s_t'{4'h1, 4'h2, 4'h3};
  end

endmodule
