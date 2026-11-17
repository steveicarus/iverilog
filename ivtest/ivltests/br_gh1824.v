// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1824.v -- Regression for GitHub issue #1824
// Reading a member of a class-handle property in a return statement is not
// supported yet. The compiler must report an error instead of crashing.

class Node;
  string s;
  Node next;
endclass

class L;
  Node head;
  function new();
    head = new;
    head.s = "x";
  endfunction
  function string first();
    return head.s;
  endfunction
endclass

module top;
  L l = new;
  initial $display("%s", l.first());
endmodule
