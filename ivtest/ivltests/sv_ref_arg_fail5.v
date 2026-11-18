// A const ref argument can not be written.

module top;
  int v;
  task automatic t(const ref int x);
    x = 1;
  endtask

  initial t(v);
endmodule
