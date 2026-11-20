// Bad method argument counts must be diagnosed without crashing.

class C;
  int q[$];
  static int sq[$];
endclass

module test;
  C c;
  initial begin
    c = new;
    c.q.sort(1);
    c.sq.reverse(1);
  end
endmodule
