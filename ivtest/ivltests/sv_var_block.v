// Check that the var keyword is supported for variable declarations in blocks

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, `"val`", val, exp); \
    failed = 1'b1; \
  end

module test;

  bit failed = 1'b0;

  initial begin
    var x;
    var [7:0] y;
    var signed [8:0] z;
    var logic [9:0] w;

    x = 1'b1;
    y = 8'd10;
    z = -8'sd1;
    w = 8'd20;

    `check(x, 1'b1)
    `check(y, 10)
    `check(z, -1)
    `check(w, 20)

    // var should default to logic and allow x state
    x = 1'bx;
    y = 8'hxx;
    z = 8'hxx;
    w = 8'hxx;

    `check(x, 1'bx)
    `check(y, 8'hxx)
    `check(z, 8'hxx)
    `check(w, 8'hxx)

    `check($bits(x), 1)
    `check($bits(y), 8)
    `check($bits(z), 9)
    `check($bits(w), 10)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
