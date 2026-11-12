// A bit-select is not a variable that can be passed by reference.

module top;
  logic [7:0] v;
  task automatic t(ref logic x);
    x = 1;
  endtask

  initial t(v[3]);
endmodule
