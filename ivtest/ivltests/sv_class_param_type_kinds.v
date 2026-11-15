// Check type parameters of a class set to an enum, a struct, a class and a
// class specialization.

typedef enum logic [1:0] { RED, GREEN, BLUE } color_t;

typedef struct packed {
  logic [3:0] hi;
  logic [7:0] lo;
} pair_t;

class Item #(int K = 1);
  function int k();
    return K;
  endfunction
endclass

class Box #(type T = int);
  T item;
endclass

module test;

  Box #(color_t) c;
  Box #(pair_t) p;
  Box #(Item) i;
  Box #(Item #(4)) i4;
  Box #(.T(Item #(4))) j4;
  Item #(4) it;
  pair_t pv;
  bit failed = 0;

  initial begin
    c = new; p = new; i = new; i4 = new;
    c.item = BLUE;
    p.item = 12'habc;
    if (c.item != BLUE || $bits(c.item) != 2) failed = 1;
    pv = p.item;
    if (pv.hi != 4'ha || $bits(p.item) != 12) failed = 1;

    i.item = new;
    it = new;
    i4.item = it;
    j4 = i4;
    it = j4.item;
    if (it.k() != 4) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end

endmodule
