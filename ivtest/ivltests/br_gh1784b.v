// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1784b.v - Regression for GitHub issue #1784
// A static class property initializer has no object, so it cannot call a
// non-static method. The compiler must report an error instead of aborting.

class C;
  function int g();
    return 1;
  endfunction

  static int v = g();   // error: g is not a static method
endclass

module top;
  initial $display("FAILED: this must not compile");
endmodule
