// Check methods on a static class-handle property through an object handle.

class Inner;
  int v;
  function new(int x); v = x; endfunction
  function int get(); return v; endfunction
  task put(input int x); v = x; endtask
endclass

class Outer;
  static Inner other;
endclass

module test;

  Outer o;
  Inner i;
  reg failed;

  `define check(val, exp) \
    if (val !== exp) begin \
      $display("FAILED(%0d). '%s' expected %0d, got %0d", `__LINE__, \
               `"val`", exp, val); \
      failed = 1'b1; \
    end

  initial begin
    failed = 1'b0;
    i = new(42);
    Outer::other = i;
    o = new;

    `check(o.other.get(), 42);
    o.other.put(43);
    `check(Outer::other.get(), 43);
    `check(o.other.get(), 43);

    if (!failed)
      $display("PASSED");
  end

endmodule
