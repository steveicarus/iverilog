// Check that type parameter declaration names can shadow visible type identifiers.

typedef int TP;

module test;

  reg failed;

  parameter type TP = logic [3:0];

  TP type_param_value;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    type_param_value = 4'hc;

    `check($bits(type_param_value), 4, "type parameter name did not hide typedef");
    `check(type_param_value, 4'hc, "type parameter value mismatch");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
