// Check that logic class properties are initialized to 'x.

module test;

   bit failed = 1'b0;

  `define check(val, exp) do \
    if (val != exp) begin \
      $display("FAILED(%0d). '%s' expected %d, got %d", `__LINE__, `"val`", exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    logic [31:0] x;
  endclass

  C c;

  initial begin
    c = new;
    `check(c.x, 32'hxx);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
