// Check class parameters without a default value: value and type parameters,
// a class extending one of them with its own parameter, a class that holds
// a handle of itself, and class scope access to the parameters.

class Stack #(type T, int DEPTH);
  T items[$];
  static int limit = DEPTH;
  localparam int TWICE = DEPTH * 2;

  function void push(T x);
    if (items.size() < DEPTH)
      items.push_back(x);
  endfunction

  function T pop();
    return items.pop_back();
  endfunction

  function int size();
    return items.size();
  endfunction
endclass

class Counted #(type U) extends Stack #(U, 3);
  U last;

  function void push2(U x);
    push(x);
    last = x;
  endfunction
endclass

class Node #(int W);
  logic [W-1:0] value;
  Node #(W) next;
endclass

module test;

  Stack #(int, 4) si;
  Stack #(.T(logic [7:0]), .DEPTH(2)) ss;
  Counted #(byte) cb;
  Node #(8) n;

  localparam int D = Stack#(int, 5)::TWICE;

  bit failed = 0;

  initial begin
    si = new;
    ss = new;
    cb = new;
    n = new;

    for (int i = 0; i < 6; i++)
      si.push(i);
    ss.push(8'hA1);
    ss.push(8'hB2);
    ss.push(8'hC3);
    cb.push2(8'd7);

    if (si.size() != 4 || si.pop() != 3) failed = 1;
    if (ss.size() != 2 || ss.pop() != 8'hB2) failed = 1;
    if (cb.size() != 1 || cb.last != 7 || $bits(cb.last) != 8) failed = 1;
    if (si.limit != 4 || ss.limit != 2 || cb.limit != 3) failed = 1;
    if (D != 10 || $bits(n.value) != 8) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
