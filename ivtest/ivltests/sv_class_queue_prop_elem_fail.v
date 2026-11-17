// Check that unsupported element assignments and selects of queue and
// dynamic array class properties are reported as errors.

module test;

  class C;
    int q[$];
    int d[];
    real r[$];
    string s[$];
  endclass

  class N;
    C b;
  endclass

  C c;
  N n;
  int i;

  initial begin
    c = new;
    c.q[0] <= 9;
    c.d[0] <= 3;
    c.r[0] <= 9.0;
    c.s[0] <= "a";
    c.q[1:0] = 1;
    c.q[i+:2] = 1;
    i = c.q[1:0];
    c.q[0] += 1;
    c.d[$] = 1;
    c.d[$+1] = 1;
    c.q[0][1] = 1;
    n.b.q[$] = 1;
    n.b.q[$+1] = 1;
    i = c.d[$-1];
  end

endmodule
