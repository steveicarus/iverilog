// Check that parameter declaration names can shadow visible type identifiers.

typedef int L;
typedef int P;
typedef int Q;
typedef int R;

module test;

  reg failed;

  parameter P = 7, R = 13;
  localparam Q = 11, L = 17;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(P, 7, "parameter name did not hide typedef");
    `check(R, 13, "parameter list continuation did not hide typedef");
    `check(Q, 11, "localparam name did not hide typedef");
    `check(L, 17, "localparam list continuation did not hide typedef");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
