// Check package import ordering for types in nested scopes.

package p1;
  typedef logic [1:0] type_t;
endpackage

package p2;
  typedef logic [3:0] type_t;
endpackage

module test;

  import p1::type_t;

  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin : b
    type_t value_before;
    import p2::type_t;
    type_t value_after;

    failed = 1'b0;

    `check($bits(value_before), 2);
    `check($bits(value_after), 4);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
