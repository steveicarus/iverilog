// A ref argument that is an unpacked array is not supported yet.

module top;
  int v[4];
  task automatic t(ref int x[4]);
    x[0] = 1;
  endtask

  initial t(v);
endmodule
