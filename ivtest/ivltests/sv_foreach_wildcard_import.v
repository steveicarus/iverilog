// Check that foreach resolves wildcard-imported arrays and declares its loop
// variables without resolving matching wildcard imports.

package p;
  reg [31:0] A[0:2];
  integer i;
endpackage

package q;
  integer i;
endpackage

module test;

  import p::*;
  import q::*;

  reg failed;
  integer count;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    count = 0;

    foreach (A[i]) begin
      count = count + 1;
    end

    `check(count, 3);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
