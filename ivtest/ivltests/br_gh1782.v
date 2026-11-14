// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1782.v - Regression for GitHub issue #1782
// name() on an enum-typed parameter.
//
// Before the fix in elab_expr.cc, this failed with:
//   sorry: Don't know how to handle method 'name()' for parameter of type:
//   error: Object p.MODE has no method "name(...)".

package p;
  typedef enum int { NONE, FAST, ACCURATE } m_t;
  localparam m_t MODE = FAST;
endpackage
module top;
  typedef enum logic [1:0] { A = 0, B = 1, C = 3 } e_t;
  localparam p::m_t LOCAL_MODE = p::ACCURATE;
  localparam e_t E0 = A;
  localparam e_t ENOMATCH = e_t'(2);   // not a named value

  initial begin
    string s1, s2, s3, s4, s5;
    s1 = p::MODE.name();
    s2 = LOCAL_MODE.name();
    s3 = E0.name();
    s4 = ENOMATCH.name();
    s5 = p::MODE.name();

    $display("%s: %s", p::MODE.name(), (p::MODE.name() == "FAST") ? "PASS" : "FAIL");
    if (s1 == "FAST" && s2 == "ACCURATE" && s3 == "A" && s4 == "" &&
        (p::MODE.name() == "FAST"))
      $display("PASSED");
    else
      $display("FAILED: '%s' '%s' '%s' '%s'", s1, s2, s3, s4);
  end
endmodule
