// An element of an unpacked array as the actual of a ref argument is not
// supported yet.
module top;
  int v[4];
  task automatic t(ref int x);
    x = 1;
  endtask
  initial t(v[1]);
endmodule
