// An event of a ref argument is not supported yet.

module top;
  int v;
  task automatic t(ref int x);
    @(x);
    wait (x == 1);
  endtask

  initial t(v);
endmodule
