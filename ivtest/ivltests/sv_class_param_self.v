// Check that inside a parameterized class the class name refers to the
// current specialization.

class Node #(type T = int);
  T val;
  Node next;
  Node #(T) prev;

  function new(T v);
    val = v;
  endfunction

  function Node copy();
    Node tmp = new(val);
    return tmp;
  endfunction
endclass

module test;

  Node #(real) a, b, c, tmp;
  Node #(byte) d, e;
  bit failed = 0;

  initial begin
    a = new(1.5);
    b = new(2.5);
    a.next = b;
    b.prev = a;
    c = a.copy();
    d = new(-2);

    c.val = 3.5;
    if (a.val != 1.5 || c.val != 3.5) failed = 1;
    tmp = a.next;
    if (tmp.val != 2.5) failed = 1;
    tmp = b.prev;
    if (tmp.val != 1.5) failed = 1;
    e = d.copy();
    if ($bits(e.val) != 8 || e.val != -2) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
