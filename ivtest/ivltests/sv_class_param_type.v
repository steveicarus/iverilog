// Check type parameters of a parameterized class, used as the type of a
// property, a method argument and a method return value, including a class
// declared in a package and a type parameter that is itself a class.

package pkg;
  class Box #(type T = int);
    T item;

    function new(T x);
      item = x;
    endfunction

    function T get();
      return item;
    endfunction
  endclass
endpackage

class Item;
  int id;
endclass

class Pair #(type K = int, type V = K);
  K key;
  V val;
endclass

module test;

  pkg::Box #(real) r;
  pkg::Box #(logic [11:0]) l;
  pkg::Box i;
  pkg::Box #(Item) h;
  Pair #(byte) p;
  Pair #(.V(string)) q;
  Item it, it2;
  bit failed = 0;

  initial begin
    r = new(1.25);
    l = new(12'habc);
    i = new(-3);
    it = new;
    it.id = 7;
    h = new(it);
    p = new;
    q = new;
    q.val = "text";

    if (r.get() != 1.25) failed = 1;
    if ($bits(l.get()) != 12 || l.get() !== 12'habc) failed = 1;
    if ($bits(i.get()) != 32 || i.get() != -3) failed = 1;
    it2 = h.get();
    it.id = 9;
    if (it2.id != 9) failed = 1;
    if ($bits(p.key) != 8 || $bits(p.val) != 8) failed = 1;
    if ($bits(q.key) != 32 || q.val != "text") failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
