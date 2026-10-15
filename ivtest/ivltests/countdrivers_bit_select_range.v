// Check that $countdrivers uses the selected bit for all declaration ranges.

module M #(parameter LEFT = 3, RIGHT = 0)(output reg failed);
  localparam STEP = LEFT >= RIGHT ? 1 : -1;
  localparam B0 = RIGHT;
  localparam B1 = RIGHT + STEP;
  localparam B2 = RIGHT + 2*STEP;
  localparam B3 = LEFT;

  wire [LEFT:RIGHT] plain, strength, multiple, switched, other;
  integer result, forced, total, zeros, ones, unknowns;

  assign plain = 4'b10xz;
  assign (weak0, strong1) strength = 4'b10xz;
  assign multiple = 4'b10xz;
  assign multiple = 4'bzz00;
  assign (weak0, strong1) other = 4'b10xz;
  tran t[LEFT:RIGHT](switched, other);

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d) %m. '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  `define count(net, f, n0, n1, nx) \
    result = $countdrivers(net, forced, total, zeros, ones, unknowns); \
    `check(result, ((n0 + n1 + nx) > 1)); \
    `check(forced, f); \
    `check(total, (n0 + n1 + nx)); \
    `check(zeros, n0); \
    `check(ones, n1); \
    `check(unknowns, nx);

  initial begin
    failed = 1'b0;
    #1;

    `count(plain[B0], 0, 0, 0, 0);
    `count(plain[B1], 0, 0, 0, 1);
    `count(plain[B2], 0, 1, 0, 0);
    `count(plain[B3], 0, 0, 1, 0);

    `count(strength[B0], 0, 0, 0, 0);
    `count(strength[B1], 0, 0, 0, 1);
    `count(strength[B2], 0, 1, 0, 0);
    `count(strength[B3], 0, 0, 1, 0);

    `count(multiple[B0], 0, 1, 0, 0);
    `count(multiple[B1], 0, 1, 0, 1);
    `count(multiple[B2], 0, 1, 0, 0);
    `count(multiple[B3], 0, 0, 1, 0);

    `count(switched[B0], 0, 0, 0, 0);
    `count(switched[B1], 0, 0, 0, 1);
    `count(switched[B2], 0, 1, 0, 0);
    `count(switched[B3], 0, 0, 1, 0);

    force plain[B2] = 1'b1;
    force strength[B2] = 1'b1;
    force multiple[B2] = 1'b1;
    `count(plain[B2], 1, 1, 0, 0);
    `count(strength[B2], 1, 1, 0, 0);
    `count(multiple[B2], 1, 1, 0, 0);
    `count(multiple[B1], 0, 1, 0, 1);

    release plain[B2];
    release strength[B2];
    release multiple[B2];
    `count(plain[B2], 0, 1, 0, 0);
    `count(strength[B2], 0, 1, 0, 0);
    `count(multiple[B2], 0, 1, 0, 0);
  end

  `undef count
  `undef check
endmodule

module test;
  wire [7:0] failed;

  M #(3, 0) m0(failed[0]);
  M #(0, 3) m1(failed[1]);
  M #(6, 3) m2(failed[2]);
  M #(3, 6) m3(failed[3]);
  M #(-3, -6) m4(failed[4]);
  M #(-6, -3) m5(failed[5]);
  M #(1, -2) m6(failed[6]);
  M #(-2, 1) m7(failed[7]);

  initial begin
    #2;
    if (failed === 8'b0) begin
      $display("PASSED");
    end
  end
endmodule
