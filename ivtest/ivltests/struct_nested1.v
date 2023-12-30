// Check that the width on a nested struct member is calculated correctly

module test;

  bit failed = 1'b0;

  `define check(expr, val) do \
    if (expr !== val) begin \
      $display("FAILED(%0d): `%s`, expected %0d, got %0d", `__LINE__, `"expr`", val, expr); \
      failed = 1'b1; \
    end \
  while (0)

  typedef enum bit [15:0] {
    A, B
  } E;


  struct packed {
    struct packed {
      struct packed {
        reg [7:0][3:0] x;
        E e;
      } s2;
      int z;
    } s1;
    int y;
  } s0;

  initial begin

    `check($bits(s0), 112);
    `check($bits(s0.s1), 80);
    `check($bits(s0.s1.s2), 48);
    `check($bits(s0.s1.s2.x), 32);
    `check($bits(s0.s1.s2.e), 16);
    `check($bits(s0.s1.s2.e.next), 16);
    `check($bits(s0.s1.s2.e.prev), 16);
    `check($bits(s0.s1.s2.e.first), 16);
    `check($bits(s0.s1.s2.e.last), 16);
    `check($bits(s0.s1.s2.e.num), 32);

    `check($bits(s0[0]), 1);
    `check($bits(s0.s1[0]), 1);
    `check($bits(s0.s1.s2[0]), 1);
    `check($bits(s0.s1.s2.x[0]), 4);
    `check($bits(s0.s1.s2.x[0][0]), 1);
    `check($bits(s0.s1.s2.e[0]), 1);

    `check($bits(s0[1:0]), 2);
    `check($bits(s0.s1[1:0]), 2);
    `check($bits(s0.s1.s2[1:0]), 2);
    `check($bits(s0.s1.s2.x[1:0]), 8);
    `check($bits(s0.s1.s2.x[0][1:0]), 2);
    `check($bits(s0.s1.s2.e[1:0]), 2);

    `check($bits(s0[0+:2]), 2);
    `check($bits(s0.s1[0+:2]), 2);
    `check($bits(s0.s1.s2[0+:2]), 2);
    `check($bits(s0.s1.s2.x[0+:2]), 8);
    `check($bits(s0.s1.s2.x[0][0+:2]), 2);
    `check($bits(s0.s1.s2.e[0+:2]), 2);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
