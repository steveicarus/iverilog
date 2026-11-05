// Check that a struct member without a key and no default is an error.

module test;

  struct packed { int a; int b; } s;

  initial begin
    s = '{a: 1};
  end

endmodule
