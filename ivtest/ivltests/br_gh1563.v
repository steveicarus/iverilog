// Check that repeat loops with X/Z counts do not execute their bodies.

module test;

  reg [64:0] count;
  reg failed;

  `define check(val) \
    repeat (val) begin \
      $display("FAILED(%0d): repeat (%s) executed with count %b", \
               `__LINE__, `"val`", val); \
      failed = 1'b1; \
      disable check_counts; \
    end

  initial begin
    failed = 1'b0;
    begin : check_counts
      count = 65'bx;
      `check(count[31:0]);
      `check(count);
      count = 65'bz;
      `check(count[31:0]);
      `check(count);

      count = 65'b1x;
      `check(count[31:0]);
      `check(count);
      count = 65'b1z;
      `check(count[31:0]);
      `check(count);

      count = {1'bx, 64'd1};
      `check(count);
      count = {1'bz, 64'd1};
      `check(count);
    end
    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
