// Check that wildcard port connections use the position of `.*`.

module child(input wire source = 1'b0, input wire value = 1'b0,
             output wire result);

  assign result = value;

endmodule

module implicit_before(output wire result);

  child i_child(.source(value), .result(result), .*);

  assign value = 1'b1;

endmodule

module implicit_after(output wire result);

  child i_child(.result(result), .*, .source(value));

  assign value = 1'b1;

endmodule

module test;

  wire result_before;
  wire result_after;

  implicit_before i_before(result_before);
  implicit_after i_after(result_after);

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

    `check(result_before, 1'b1);
    `check(result_after, 1'b0);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
