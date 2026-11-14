// The actual of a ref argument must have the type of the formal.

module top;
  logic [7:0] v;
  task automatic t(ref int x);
    x = 1;
  endtask

  initial t(v);
endmodule
