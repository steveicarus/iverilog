// Check package import ordering for foreach array references.

package p;
  reg [31:0] value[0:1];
endpackage

reg [31:0] value[0:0];

module test;

  integer count_before;
  integer count_after;
  reg failed;

  initial begin
    count_before = 0;
    foreach (value[i]) begin
      count_before = count_before + 1;
    end
  end

  import p::value;

  initial begin
    count_after = 0;
    foreach (value[i]) begin
      count_after = count_after + 1;
    end
  end

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    #1;

    `check(count_before, 1);
    `check(count_after, 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
