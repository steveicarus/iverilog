// Testcase for Issue #823 on Github

module test;
  struct packed {struct packed {logic a;} t;} u;
  assign u.t.a = 1;
  assign u.t.c = 1;
endmodule
