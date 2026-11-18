// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// Check that the built-in class method srandom(seed) is accepted and
// ignored (a warning is printed), and that it does not disturb the object.
// GitHub issue #439.
class C;
  int x;
  function new();
    x = 5;
  endfunction
endclass

module test;
  C c = new;
  initial begin
    c.srandom(42);
    if (c.x == 5) $display("PASSED");
    else $display("FAILED");
  end
endmodule
