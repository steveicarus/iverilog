// Copyright (c) 2026 Srinivasan Venkataramanan (AsFigo, https://www.asfigo.com)
// br_gh1855.v - Regression for GitHub issue #1855
// An extern method whose return type is a specialization of a parameterized
// class (N #(int), N #(T)) must be definable outside the class body.
//
// Before the fix, the out-of-body definitions of a and c gave a syntax error.

class N #(type T);
  T v;
endclass

// a: plain class, parameterized-class return type
class A;
  extern function N #(int) get_node();
endclass

function N #(int) A::get_node();
  N #(int) n;
  n = new;
  n.v = 5;
  return n;
endfunction

// b: parameterized class, plain return type (control, worked before)
class B #(type T);
  T v;
  extern function int get_int();
endclass

function int B::get_int();
  return 7;
endfunction

// c: parameterized class, parameterized-class return type
class L #(type T);
  N #(T) head;
  extern function N #(T) get_node();
endclass

function N #(T) L::get_node();
  return head;
endfunction

module main;
  int errors = 0;
  A a;
  B #(int) b;
  L #(int) l;
  N #(int) n;

  initial begin
    a = new;
    n = a.get_node();
    if (n.v !== 5) begin $display("FAILED: a.get_node() v=%0d", n.v); errors += 1; end

    b = new;
    if (b.get_int() !== 7) begin $display("FAILED: b.get_int()"); errors += 1; end

    l = new;
    n = new;
    n.v = 9;
    l.head = n;
    n = l.get_node();
    if (n.v !== 9) begin $display("FAILED: l.get_node() v=%0d", n.v); errors += 1; end

    if (errors == 0) $display("PASSED");
  end
endmodule
