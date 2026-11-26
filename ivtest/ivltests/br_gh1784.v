// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1784.v - Regression for GitHub issue #1784
// A static class property initialized by calling a static method of the same
// class, and a static method calling another static method.
//
// Before the fix, the compiler aborted in elab_expr.cc with a failed
// assertion on method_scope.

class C;
  static function bit f();
    return 1;
  endfunction

  static function int g();
    return 40;
  endfunction

  static function int k(int a, int b);
    return a * 10 + b;
  endfunction

  static function int h();
    return g() + 2;        // static method calling a static method
  endfunction

  static bit registered = f();   // static initializer calling a static method
  static int value = g();
  static int w = k(5, 7);        // static initializer with arguments
endclass

module top;
  initial begin
    C o;
    o = new;
    if (o.registered === 1'b1 && o.value === 40 && o.w === 57 && o.h() === 42)
      $display("PASSED");
    else
      $display("FAILED: registered=%b value=%0d w=%0d h=%0d",
               o.registered, o.value, o.w, o.h());
  end
endmodule
