// Check that a compilation-unit object takes precedence over an enclosing
// instance when resolving a member reference.

typedef struct packed {
  integer value;
} object_t;

object_t object = 11;

module holder;

  integer value = 22;

endmodule

module child;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;

    `check(object.value, 11)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule

module test;

  holder object();
  child child_instance();

endmodule
