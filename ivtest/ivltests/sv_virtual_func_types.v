// Check the argument and return types of virtual functions called
// through a handle of the base class, including void functions, default
// argument values and a class handle result.

module test;

  class Item;
    int id;
    function new(int i = 0);
      id = i;
    endfunction
  endclass

  class SubItem extends Item;
    function new(int i = 0);
      super.new(i + 100);
    endfunction
  endclass

  class Base;
    int last;
    virtual function bit [7:0] vec(bit [7:0] a, bit [7:0] b);
      return a + b;
    endfunction
    virtual function int i32(int a, int b = 5);
      return a - b;
    endfunction
    virtual function real r(real a, int n);
      return a * n;
    endfunction
    virtual function string s(string a, string b);
      return {a, b};
    endfunction
    virtual function Item item(int n);
      Item it = new(n);
      return it;
    endfunction
    virtual function void set(int v, string tag = "base");
      last = v;
    endfunction
  endclass

  class Derived extends Base;
    function bit [7:0] vec(bit [7:0] a, bit [7:0] b);
      return a ^ b;
    endfunction
    function int i32(int a, int b = 5);
      return a * b;
    endfunction
    function real r(real a, int n);
      return a / n;
    endfunction
    function string s(string a, string b);
      return {b, a};
    endfunction
    // A covariant return type
    function SubItem item(int n);
      SubItem it = new(n);
      return it;
    endfunction
    function void set(int v, string tag = "derived");
      last = v + 1000;
    endfunction
  endclass

  // An unpacked array argument is fine if nothing overrides the function
  class Arr;
    virtual function int sum(int a[3]);
      return a[0] + a[1] + a[2];
    endfunction
  endclass

  Arr arr;
  int v[3] = '{1, 2, 3};
  Base b;
  Derived d;
  Item it;
  int errors = 0;

  initial begin
    arr = new;
    if (arr.sum(v) != 6) errors += 1;

    b = new;
    if (b.vec(8'h0f, 8'h01) !== 8'h10) errors += 1;
    if (b.i32(10, 3) != 7) errors += 1;
    if (b.i32(10) != 5) errors += 1;
    if (b.r(6.0, 2) != 12.0) errors += 1;
    if (b.s("ab", "cd") != "abcd") errors += 1;
    it = b.item(3);
    if (it.id != 3) errors += 1;
    b.set(4);
    if (b.last != 4) errors += 1;

    d = new;
    b = d;
    if (b.vec(8'h0f, 8'h01) !== 8'h0e) errors += 1;
    if (b.i32(10, 3) != 30) errors += 1;
    if (b.i32(10) != 50) errors += 1;
    if (b.r(6.0, 2) != 3.0) errors += 1;
    if (b.s("ab", "cd") != "cdab") errors += 1;
    it = b.item(3);
    if (it.id != 103) errors += 1;
    b.set(4);
    if (b.last != 1004) errors += 1;
    // The argument expressions may call virtual functions too
    if (b.i32(b.i32(2, 3), b.vec(1, 3)) != 12) errors += 1;

    if (errors == 0)
      $display("PASSED");
    else
      $display("FAILED: %0d errors", errors);
  end

endmodule
