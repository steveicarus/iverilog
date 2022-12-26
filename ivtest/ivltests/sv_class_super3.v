// Check that `super` keyword can be used to access members of the base class

class B;
  int x, y;

  task set_y;
    y = 2000;
  endtask

  function bit check_x;
    return x === 1000;
  endfunction
endclass

class C extends B;
  byte x, y;
  task set_x;
    super.x = 1000;
  endtask

  function bit check_y;
    return super.y === 2000;
  endfunction
endclass

module test;
  C c;

  initial begin
    c = new;
    c.set_x;
    c.set_y;
    if (c.check_x() && c.check_y()) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
