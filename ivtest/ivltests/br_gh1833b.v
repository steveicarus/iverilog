// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1833b.v - Regression for GitHub issue #1833
// === and !== between two class handles, and a function call result compared
// with null, must compile and give the right answers.
//
// Before the fix, tgt-vvp/eval_condit.c reported "SORRY: Compare class
// handles not implemented" for these forms.

class C;
  int v;
endclass

class H;
  C c;
  function C get();
    return c;
  endfunction
endclass

module main;
  C a, b, d, n;
  H h;
  int errors = 0;

  initial begin
    a = new;
    b = new;
    d = a;
    h = new;
    h.c = a;

    // handle vs handle
    if (!(a === d)) begin $display("FAILED: a === d"); errors += 1; end
    if (a !== d)    begin $display("FAILED: a !== d"); errors += 1; end
    if (a === b)    begin $display("FAILED: a === b"); errors += 1; end
    if (!(a !== b)) begin $display("FAILED: a !== b"); errors += 1; end

    // two null handles are equal
    if (!(n === n)) begin $display("FAILED: n === n"); errors += 1; end

    // handle vs null still works
    if (a === null) begin $display("FAILED: a === null"); errors += 1; end
    if (null === a) begin $display("FAILED: null === a"); errors += 1; end
    if (!(n === null)) begin $display("FAILED: n === null"); errors += 1; end
    if (!(null === n)) begin $display("FAILED: null === n"); errors += 1; end

    // call result vs null
    if (h.get() === null) begin $display("FAILED: get() === null"); errors += 1; end
    if (null === h.get()) begin $display("FAILED: null === get()"); errors += 1; end
    if (!(null !== h.get())) begin $display("FAILED: null !== get()"); errors += 1; end
    if (!(h.get() !== null)) begin $display("FAILED: get() !== null"); errors += 1; end

    // call result vs handle
    if (!(h.get() === a)) begin $display("FAILED: get() === a"); errors += 1; end
    if (h.get() === b)    begin $display("FAILED: get() === b"); errors += 1; end

    h.c = null;
    if (!(h.get() === null)) begin $display("FAILED: null get() === null"); errors += 1; end
    if (!(null === h.get())) begin $display("FAILED: null null === get()"); errors += 1; end

    if (errors == 0) $display("PASSED");
  end
endmodule
