// A ref argument needs an automatic subroutine.

module top;
  int v;
  task static t(ref int x);
    x = 1;
  endtask

  initial t(v);
endmodule
