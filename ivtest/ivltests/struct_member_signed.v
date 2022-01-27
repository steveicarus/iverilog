// Tests that the signedness for struct members is handled correctly

module test;

  struct packed {
    logic [15:0] x;
    logic signed [15:0] y;
  } s;

  bit failed = 1'b0;

  `define check(x) \
    if (!(x)) begin \
      $display("FAILED: ", `"x`"); \
      failed = 1'b1; \
    end

  initial begin
    s.x = -1;
    s.y = -1;

    `check(!$is_signed(s.x));
    `check($is_signed(s.y));

    // These evaluate as signed
    `check($signed(s.x) < 0);
    `check(s.y < 0);

    // These all evaluate as unsigned
    `check(s.x > 0);
    `check(s.y[15:0] > 0)
    `check({s.y} > 0)
    `check($unsigned(s.y) > 0)
    `check(s.y > 16'h0)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
