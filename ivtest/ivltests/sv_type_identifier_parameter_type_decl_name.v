// Check that typed parameter declaration names can shadow visible type identifiers.

typedef int P;
typedef logic [7:0] T;
typedef logic [6:0] U;

module test;

  reg failed;

  parameter int P = 13;
  parameter T typed_value = 8'ha5;
  parameter T T = 8'h3c;
  parameter U u0 = 7'h2a, U = 7'h15;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(P, 13, "typed parameter name did not hide typedef");
    `check($bits(typed_value), 8, "typed parameter width mismatch");
    `check(typed_value, 8'ha5, "typed parameter value mismatch");
    `check($bits(T), 8, "type-name parameter did not keep typedef type");
    `check(T, 8'h3c, "type-name parameter value mismatch");
    `check($bits(u0), 7, "parameter list first declaration did not keep typedef type");
    `check(u0, 7'h2a, "parameter list first value mismatch");
    `check($bits(U), 7, "parameter list continuation did not allow typedef name as parameter name");
    `check(U, 7'h15, "parameter list continuation value mismatch");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
