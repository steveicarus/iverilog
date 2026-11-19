// Check that constant forces affect only the selected bits inside a net.

module test;
  wire [7:0] target = 8'ha0;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    force target[2:-1] = 4'b1010;
    #1;
    `check(target, 8'ha5);
    force target[9:6] = 4'b1101;
    #1;
    `check(target, 8'h65);

    force target[-2:-5] = 4'b1111;
    force target[12:9] = 4'b1111;
    #1;
    `check(target, 8'h65);
    release target;
    #1;
    `check(target, 8'ha0);
    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
