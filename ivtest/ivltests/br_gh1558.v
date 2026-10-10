// Check constant enum casts in contexts with different widths or signedness.

module test;

  typedef enum logic [1:0] {A, B, C, D} e_t;
  typedef enum logic signed [1:0] {NEG = -2} signed_e_t;

  e_t e;
  int value;
  bit failed = 1'b0;

  `define check(val, exp) \
    if ((val) !== (exp)) begin \
      $display("FAILED(%0d): '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    e = e_t'(6);
    `check(e, C);

    value = e_t'(2);
    `check(value, 2);

    value = e_t'(6);
    `check(value, 2);
    `check(int'(e_t'(6)), 2);
    `check(8'(e_t'(6)), 8'd2);

    value = signed_e_t'(2);
    `check(value, -2);

    `check(signed_e_t'(2) | 2'b00, 2'b10);
    `check(signed_e_t'(2) == 2'b10, 1'b1);
    `check(1'b1 ? signed_e_t'(2) : 2'b00, 2'b10);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
