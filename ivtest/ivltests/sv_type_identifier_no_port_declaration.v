// Check that a visible type selects a declaration unless ports are specified.

module M;
  localparam int VALUE = 1;
endmodule

module test;
  typedef logic [3:0] M;

  reg failed;
  M value; // Declares variable.
  M values [1:0]; // Declares variable.

  M i_m(); // Instantiates module.
  M i_m1 [1:0], i_m2(), i_m3 [1:0]; // Instantiates modules.

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    value = 4'ha;
    values[0] = 4'h3;
    values[1] = 4'hc;

    `check($bits(value), 4);
    `check(value, 4'ha);
    `check($bits(values[0]), 4);
    `check(values[0], 4'h3);
    `check(values[1], 4'hc);
    `check(i_m.VALUE, 1);
    `check(i_m1[0].VALUE, 1);
    `check(i_m1[1].VALUE, 1);
    `check(i_m2.VALUE, 1);
    `check(i_m3[0].VALUE, 1);
    `check(i_m3[1].VALUE, 1);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
