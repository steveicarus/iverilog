// Regression test for GitHub issue #670.
// Check that a class function can have the same name as the class.

module test;

  reg failed;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  class test;
    int value;

    function void test();
      value = 32'd42;
    endfunction
  endclass

  initial begin
    test tst;

    failed = 1'b0;
    tst = new;
    tst.test();

    `check(tst.value, 32'd42, "Class function name did not hide class name");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
