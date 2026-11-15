// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1786.v - Regression for GitHub issue #1786
// A call through a base class handle to a virtual function or task whose
// base body is empty must still dispatch to the derived override.
//
// Before the fix in elaborate.cc, the call to the empty virtual method was
// elided at compile time and the override never ran.

class B;
  int cnt_e, cnt_te, cnt_s;
  virtual function void e(); endfunction              // empty
  virtual task          te(); endtask                 // empty
  virtual function void s(); int x; x = 1; endfunction // non-empty
  function void         n(); endfunction              // empty, not virtual
endclass

class M extends B;
  virtual function void e(); endfunction              // empty override
endclass

class D extends M;
  virtual function void e();  cnt_e  += 1; endfunction
  virtual task          te(); cnt_te += 1; endtask
  virtual function void s();  cnt_s  += 1; endfunction
endclass

module top;
  initial begin
    B b;
    D d;
    d = new;
    b = d;
    b.e();    // empty in B and M, counted in D
    b.te();
    b.s();
    b.n();    // empty non-virtual method, still fine
    d.e();    // call through the derived handle
    if (d.cnt_e === 2 && d.cnt_te === 1 && d.cnt_s === 1)
      $display("PASSED");
    else
      $display("FAILED: cnt_e=%0d (expect 2) cnt_te=%0d (expect 1) cnt_s=%0d (expect 1)",
               d.cnt_e, d.cnt_te, d.cnt_s);
  end
endmodule
