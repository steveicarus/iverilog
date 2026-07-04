// Check that enum item names can shadow visible type identifiers.

typedef int T;
typedef int U;
typedef int V;

module test;

  reg failed;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  enum {
    T = 3,
    U[2] = 5,
    V[3:4] = 9
  } e;

  initial begin
    failed = 1'b0;

    e = T;

    `check(e, 3, "Enum item name did not hide typedef");
    `check(U0, 5, "Enum item sequence name did not hide typedef");
    `check(U1, 6, "Enum item sequence value mismatch");
    `check(V3, 9, "Enum item range name did not hide typedef");
    `check(V4, 10, "Enum item range value mismatch");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
