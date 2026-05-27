// Check indexed access to static class array properties.

class C;

  static logic [1:0][3:0] A[2][3];

  task t(output integer value);
    A[0][0] = 1;
    this.A[1][2] = 6;
    value = A[0][0] + this.A[1][2];
  endtask

endclass

module test;

  reg failed;
  integer value;
  C c = new;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    c.t(value);

    `check(value, 7);
    `check(c.A[0][0], 1);
    `check(c.A[1][2], 6);

    c.A[1][1] = 20;
    `check(c.A[1][1], 20);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
