// Check that parameter port declaration names can shadow visible type identifiers.

typedef int P;
typedef int Q;
typedef int R;
typedef logic [7:0] T;
typedef int TP;

package p;
  typedef logic [5:0] PT;
endpackage

`define check(value, expected, error) \
  if ((value) !== (expected)) begin \
    $display("FAILED(%0d). %s", `__LINE__, error); \
    $display("  expected %0h, got %0h", expected, value); \
    failed = 1'b1; \
  end

module M #(
  parameter int P = 5,
  Q = 9,
  int R = 13,
  T typed_value = 8'ha5,
  T T = 8'h3c,
  p::PT pkg_value = 6'h2a,
  parameter type TP = logic [5:0]
) (output reg failed);

  TP type_param_value;

  initial begin
    failed = 1'b0;
    type_param_value = 6'h15;

    `check(P, 5, "parameter port typed value mismatch");
    `check(Q, 9, "parameter port untyped continuation mismatch");
    `check(R, 13, "parameter port atomic type continuation mismatch");
    `check($bits(typed_value), 8, "parameter port typedef type continuation width mismatch");
    `check(typed_value, 8'ha5, "parameter port typedef type continuation value mismatch");
    `check($bits(T), 8, "parameter port type-name continuation did not keep typedef type");
    `check(T, 8'h3c, "parameter port type-name continuation value mismatch");
    `check($bits(pkg_value), 6, "parameter port package type continuation width mismatch");
    `check(pkg_value, 6'h2a, "parameter port package type continuation value mismatch");
    `check($bits(type_param_value), 6, "parameter port type parameter mismatch");
    `check(type_param_value, 6'h15, "parameter port type parameter value mismatch");
  end

endmodule

module N #(P = 3, T typed_value = 8'h5a) (output reg failed);

  initial begin
    failed = 1'b0;

    `check(P, 3, "omitted parameter keyword mismatch");
    `check($bits(typed_value), 8, "omitted parameter keyword typedef width mismatch");
    `check(typed_value, 8'h5a, "omitted parameter keyword typedef value mismatch");
  end

endmodule

module test;

  reg failed;
  wire failed_m;
  wire failed_n;

  M i_m(failed_m);
  N i_n(failed_n);

  initial begin
    failed = 1'b0;

    #1;

    `check(failed_m, 1'b0, "parameter port module failed");
    `check(failed_n, 1'b0, "omitted parameter keyword module failed");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
