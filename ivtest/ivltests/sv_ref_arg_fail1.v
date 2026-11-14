// The actual of a ref argument must not be a net.

module top;
  wire w;
  task automatic t(ref logic x);
    x = 1;
  endtask

  initial t(w);
endmodule
