// Check that restricted class type parameter defaults are supported.

class C0;
  bit [7:0] value;
endclass

module M #(
  parameter type class T = C0
);
  T x;
endmodule

module test;

`define check(val, exp) \
  if (val !== exp) begin \
    $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, `"val`", exp, val); \
    failed = 1'b1; \
  end

  bit failed = 1'b0;

  M i_m();

  initial begin
    i_m.x = new;

    `check($bits(i_m.x.value), 8)

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
