// The actual of a ref argument must not be an expression.

module top;
  int v;
  task automatic t(ref int x);
    x = 1;
  endtask

  initial t(v + 1);
endmodule
