// Check assignments to class properties in l-value concatenations.

class C;
  logic [7:0] value;
endclass

class D;
  C child;
endclass

module test;

  bit failed = 1'b0;
  logic other;
  C c;
  D d;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %h, got %h", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    c = new;
    d = new;
    d.child = c;

    // Storing the other concatenation components is not yet supported.
    {other, c.value} = 9'h134;
    `check(c.value, 8'h34);

    {other, d.child.value} = 9'h156;
    `check(c.value, 8'h56);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
