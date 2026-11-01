// Check that parameters declared without the parameter keyword can be
// overridden after a localparam declaration in a preceding module.

module seed;
  localparam L = 0;
endmodule

module M #(P = 1);
  localparam L = 0;
endmodule

module N #(integer P = 1);
endmodule

module test;

  bit failed = 1'b0;

  M #(.P(7)) m();
  N #(.P(9)) n();

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    `check(m.P, 7);
    `check(n.P, 9);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
