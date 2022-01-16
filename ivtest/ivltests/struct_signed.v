// Tests that packed structs can have signed/unsigned modifier and that signed
// packed structs when used as a primary behave correctly.

module test;

  // Unsigned by default
  struct packed {
    logic [15:0] x;
  } s1;

  // Explicitly unsigned
  struct packed unsigned {
    logic [15:0] x;
  } s2;

  // Explicitly signed
  struct packed signed {
    logic [15:0] x;
  } s3;


  bit failed = 1'b0;

  `define check(x) \
    if (!(x)) begin \
      $display("FAILED: ", `"x`"); \
      failed = 1'b1; \
    end

  initial begin
    s1 = -1;
    s2 = -1;
    s3 = -1;

    `check(!$is_signed(s1));
    `check(!$is_signed(s2));
    `check($is_signed(s3));

    // These evaluate as signed
    `check($signed(s1) < 0);
    `check($signed(s2) < 0);
    `check(s3 < 0);

    // These all evaluate as unsigned
    `check(s1 > 0);
    `check(s2 > 0);
    `check(s3.x > 0)
    `check(s3[15:0] > 0)
    `check({s3} > 0)
    `check($unsigned(s3) > 0)
    `check(s3 > 16'h0)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
