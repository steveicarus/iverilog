// Check that nested structs and unions can reuse member names.

module test;
  bit failed = 1'b0;

  struct packed {
    struct packed {
      reg [3:0] value;
    } left;
    union packed {
      reg [3:0] value;
      reg [3:0] other;
    } right;
    reg [3:0] value;
  } data;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    data = 12'h123;
    `check(data.left.value, 4'h1);
    `check(data.right.value, 4'h2);
    `check(data.right.other, 4'h2);
    `check(data.value, 4'h3);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
