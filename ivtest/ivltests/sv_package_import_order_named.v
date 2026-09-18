// Check that implicit named port connections activate wildcard imports.

package p;
  parameter [31:0] value = 2;
endpackage

parameter [31:0] value = 1;

module child(input wire [31:0] value, output wire [31:0] result);

  assign result = value;

endmodule

module test;

  wire [31:0] result_before;
  wire [31:0] result_after;

  child i_before(.value, .result(result_before));
  import p::*;
  child i_after(.value, .result(result_after));

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    #1;

    `check(result_before, 32'd1);
    `check(result_after, 32'd2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
