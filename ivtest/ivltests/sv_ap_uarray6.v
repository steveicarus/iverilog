// Check that procedural assignment of unpacked string array assignment patterns
// to multi-dimensional arrays is supported and entries are assigned in the
// right order.

module test;

  bit failed;

  `define check(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %s, got %s", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  string x[1:0][1:0];
  string y[1:0][0:1];
  string z[2][2];

  initial begin
    x = '{'{"Hello", "World"}, '{"Array", "Pattern"}};
    y = '{'{"Hello", "World"}, '{"Array", "Pattern"}};
    z = '{'{"Hello", "World"}, '{"Array", "Pattern"}};

    `check(x[0][0], "Pattern");
    `check(x[0][1], "Array");
    `check(x[1][0], "World");
    `check(x[1][1], "Hello");

    `check(y[0][0], "Array");
    `check(y[0][1], "Pattern");
    `check(y[1][0], "Hello");
    `check(y[1][1], "World");

    `check(z[0][0], "Hello");
    `check(z[0][1], "World");
    `check(z[1][0], "Array");
    `check(z[1][1], "Pattern");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
