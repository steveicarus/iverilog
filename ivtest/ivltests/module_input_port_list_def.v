// Check that it is possible to specify a default port value for each port in a
// input port declaration list.

module M (
  input [31:0] x = 1, y = 2
);

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d): %s, expected %0h got %0h", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end

  reg failed = 1'b0;

  initial begin
    `check(x, 1)
    `check(y, 2)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule

module test;

  M i_m ();

endmodule
