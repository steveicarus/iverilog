// Testcase for Issue #823 on Github

module test;
  struct packed {logic a;} s;
  assign s.a = 1;
  assign s.c = 1;
endmodule
