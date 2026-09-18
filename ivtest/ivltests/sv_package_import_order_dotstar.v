// Check package import ordering for implicit named port connections.

package p;
  integer value = 2;
endpackage

module child(input integer value = 1, output wire [31:0] result);

  assign result = value;

endmodule

module test;

  wire [31:0] result;
  child i_child(.result(result), .*);

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
    #1;

    `check(result, 1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
