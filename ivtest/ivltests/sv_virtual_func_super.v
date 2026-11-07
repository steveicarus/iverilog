// Check that super.method() runs the function of the base class instead
// of being dispatched on the class of the object, for virtual functions
// that return a value and for void functions.

module test;

  class Base;
    int log;
    virtual function int calc(int a);
      return a + 1;
    endfunction
    virtual function void note(int v);
      log = log * 10 + v;
    endfunction
    function int outer(int a);
      return calc(a);
    endfunction
  endclass

  class Mid extends Base;
    function int calc(int a);
      return super.calc(a) * 10;
    endfunction
    function void note(int v);
      super.note(v);
      super.note(v + 1);
    endfunction
  endclass

  class Leaf extends Mid;
    function int calc(int a);
      return super.calc(a) + 5;
    endfunction
    function void note(int v);
      this.super.note(v + 2);
    endfunction
  endclass

  Base b;
  Leaf l;
  int errors = 0;

  initial begin
    l = new;
    b = l;
    // Leaf -> Mid -> Base: ((a + 1) * 10) + 5
    if (b.calc(1) != 25) errors += 1;
    if (b.outer(1) != 25) errors += 1;
    b.note(1);
    // Leaf calls Mid with 3, which logs 3 and 4
    if (b.log != 34) begin
      $display("log=%0d", b.log);
      errors += 1;
    end

    if (errors == 0)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
