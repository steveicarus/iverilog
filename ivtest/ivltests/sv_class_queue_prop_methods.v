// Regression: queue-typed class properties — push_front/push_back and
// pop_front/pop_back. (VVP asm must recognize %store/prop/qf/* and
// %qpop/prop/*; opcode_table must stay lexicographically sorted.)

module test;

  bit failed = 1'b0;

  `define check(val, exp) do \
    if (val !== exp) begin \
      $display("FAILED(%0d). expected %0d, got %0d", `__LINE__, exp, val); \
      failed = 1'b1; \
    end \
  while(0)

  class C;
    int q[$];
  endclass

  C c;
  int t;

  initial begin
    c = new;
    c.q.push_back(1);
    c.q.push_front(0);
    c.q.push_back(2);
    t = c.q.pop_back();
    `check(t, 32'd2);
    `check(c.q.size(), 32'd2);
    t = c.q.pop_front();
    `check(t, 32'd0);
    `check(c.q[0], 32'd1);

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
