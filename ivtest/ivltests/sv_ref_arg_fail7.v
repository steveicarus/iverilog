// A task call must pass something for a ref argument.

module top;
  task automatic t(ref int x);
    x = 1;
  endtask

  initial t();
endmodule
