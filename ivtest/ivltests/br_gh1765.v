// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1765.v - Regression for GitHub issue #1765
// Function call on a class-typed member handle fails to elaborate.
//
// Before the fix in elab_expr.cc, calling a function through a class-typed
// member property searched the enclosing class instead of the property's
// class:
//   "error: p is not a method of class outer_t."

class inner_t;
  int base;

  function int f();
    return 42;
  endfunction

  function int add(int v);
    return base + v;
  endfunction
endclass

class outer_t;
  inner_t p;

  function int g();
    return p.f();          // function call on class-typed member handle
  endfunction

  function int h(int v);
    return p.add(v);       // same, with an argument
  endfunction
endclass

module top;
  initial begin
    outer_t o;
    o = new;
    o.p = new;
    o.p.base = 100;
    if (o.g() === 42 && o.h(5) === 105)
      $display("PASSED");
    else
      $display("FAILED: g=%0d (expect 42) h=%0d (expect 105)", o.g(), o.h(5));
  end
endmodule
