// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1853.v - Regression for GitHub issue #1853
// A class holding a specialization that names the class itself (L#(C,C)
// in C), plus a second specialization L#(C,int), must compile. It used to
// overflow the compiler's stack.

class N #(type K, type V);
  K key;
  V val;
endclass

class L #(type K, type V);
  N #(K, V) head;
endclass

class C;
  L #(C, C) g;
endclass

class probe;
  L #(C, int) a;
endclass

module main;
  C c;
  probe p;
  L #(C, C) lcc;
  L #(C, int) lci;

  initial begin
    c = new;
    p = new;
    lcc = new;
    lci = new;
    $display("PASSED");
  end
endmodule
