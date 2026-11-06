// Check that the key of a struct pattern item must be a member name.

module test;

  struct packed { logic [3:0] a; logic [3:0] b; } s;

  initial begin
    s = '{0: 4'h1, default: 4'h2};
  end

endmodule
