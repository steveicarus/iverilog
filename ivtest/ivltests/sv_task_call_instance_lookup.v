// Check that task calls in different instances of the same module are
// resolved through each enclosing instance.

module child(output reg [7:0] result);

  initial set_value(result);

endmodule

module parent_a(output wire [7:0] result);

  task set_value;
    output [7:0] result;
    result = 8'ha1;
  endtask

  child i_child(result);

endmodule

module parent_b(output wire [7:0] result);

  task set_value;
    output [7:0] result;
    result = 8'hb2;
  endtask

  child i_child(result);

endmodule

module test;

  wire [7:0] result_a;
  wire [7:0] result_b;
  reg failed;

  parent_a i_parent_a(result_a);
  parent_b i_parent_b(result_b);

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    #1;

    `check(result_a, 8'ha1)
    `check(result_b, 8'hb2)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
