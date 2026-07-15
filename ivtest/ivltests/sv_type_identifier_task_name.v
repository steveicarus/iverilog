// Check that task names can shadow visible type identifiers.

typedef int T;
typedef int U;

module test;

  reg failed;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  task T(input int value, output int result);
    result = value + 32'd1;
  endtask

  task U;
    output int result;

    result = 32'd33;
  endtask

  class C;
    task T(input int value, output int result);
      result = value + 32'd2;
    endtask

    task C(input int value, output int result);
      result = value + 32'd3;
    endtask
  endclass

  initial begin
    C c;
    int r0;
    int r1;
    int r2;
    int r3;

    failed = 1'b0;
    c = new;

    T(32'd41, r0);
    U(r1);
    c.T(32'd40, r2);
    c.C(32'd39, r3);

    `check(r0, 32'd42, "Task name did not hide typedef");
    `check(r1, 32'd33, "Non-ANSI task name did not hide typedef");
    `check(r2, 32'd42, "Class task name did not hide typedef");
    `check(r3, 32'd42, "Class task name did not hide class name");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
