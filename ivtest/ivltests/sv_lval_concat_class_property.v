// Check direct and nested class properties in concatenated l-values.

module test;

  class C;
    C child;
    bit [3:0] two;
    logic [3:0] four;
  endclass

  bit failed = 1'b0;
  logic [3:0] four;
  C obj, middle, leaf, replacement;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %b, got %b", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  function logic [7:0] replace_child();
    obj.child = replacement;
    return 8'h11;
  endfunction

  initial begin
    obj = new;
    middle = new;
    leaf = new;
    replacement = new;
    obj.child = middle;
    middle.child = leaf;
    {four, obj.child.child.two} = 8'bx1z1_x1z1;
    `check({four, leaf.two}, 8'bx1z1_0101);
    {obj.child.child.two, four} = 8'bz0x1_z0x1;
    `check({four, leaf.two}, 8'bz0x1_0001);
    {obj.two, obj.four} = 8'bx1z1_x1z1;
    `check({obj.two, obj.four}, 8'b0101_x1z1);
    {obj.two, obj.child.child.four} = 8'h12;
    {obj.two, obj.child.child.four} += 8'h23;
    `check({obj.two, leaf.four}, 8'h35);

    middle.two = 4'h4;
    replacement.two = 4'h9;
    {obj.child.two, obj.two} += replace_child();
    `check({obj.two, middle.two, replacement.two}, 12'h459);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
