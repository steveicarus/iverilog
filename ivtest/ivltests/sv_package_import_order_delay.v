// Check that a bare delay identifier activates a wildcard package import.

package p;
  parameter delay = 2;
endpackage

parameter delay = 1;

module test;

  integer before_time;
  integer after_time;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    before_time = -1;
    #delay;
    before_time = $time;
  end

  import p::*;

  initial begin
    after_time = -1;
    #delay;
    after_time = $time;
  end

  initial begin
    failed = 1'b0;
    #3;

    `check(before_time, 1);
    `check(after_time, 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
