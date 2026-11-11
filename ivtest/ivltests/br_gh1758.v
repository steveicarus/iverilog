// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1758.v — Regression for GitHub issue #1758
// Task call on a class-typed member handle elaborates incorrectly.
//
// Before fix in elaborate.cc, calling a task through a class-typed member
// property searched the enclosing class instead of the property's class:
//   "error: Can't find task put in class gen_t"

class mbx_t;
  virtual task put(input int v, output int got);
    got = v;
  endtask
endclass

class gen_t;
  mbx_t prod_outbox;

  virtual task main(output int out);
    prod_outbox.put(42, out);   // task call on class-typed member handle
  endtask
endclass

module top;
  int result;
  initial begin
    gen_t g;
    g = new;
    g.prod_outbox = new;
    g.main(result);
    if (result === 42)
      $display("PASSED");
    else
      $display("FAILED: result=%0d (expect 42)", result);
  end
endmodule
