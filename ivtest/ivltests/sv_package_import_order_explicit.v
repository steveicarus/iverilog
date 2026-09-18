// Check that an explicit package import only affects following references.

package p;
  parameter value = 2;
endpackage

parameter value = 1;

module test;

  localparam value_before = value;
  import p::value;
  localparam value_after = value;
  import p::value;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(value_before, 1);
    `check(value_after, 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
