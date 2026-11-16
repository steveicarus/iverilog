// Check that the prefix of a typed pattern must be a type.

module test;

  logic [7:0] v;
  int x;

  initial begin
    v = x'{4'h1, 4'h2};
  end

endmodule
