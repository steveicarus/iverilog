// Check assignments to dynamic array elements in l-value concatenations.

module test;

  bit failed = 1'b0;
  logic [7:0] a[];
  bit [3:0] b[];
  logic [7:0] value;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    a = new[2];
    b = new[2];

    {value, a[0]} = 16'h1234;
    `check({value, a[0]}, 16'h12_34);

    {a[1], value} = 16'h5678;
    `check({a[1], value}, 16'h56_78);

    {b[0], a[0], b[1]} = 16'bx1z0_10xz_01zx_z0x1;
    `check({b[0], a[0], b[1]}, 16'b0100_10xz_01zx_0001);

    {value[3:0], a[1]} = 12'hab7;
    `check({value, a[1]}, 16'h7a_b7);

    {a[1], value} += 16'h0102;
    `check({a[1], value}, 16'hb8_7c);

    {value, a[1]} += 16'h0102;
    `check({value, a[1]}, 16'h7d_ba);

    {value, b[1]} |= 12'b0000_0000_x0z0;
    `check({value, b[1]}, 12'h7d1);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
