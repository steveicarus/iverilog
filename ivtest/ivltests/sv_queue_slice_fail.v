// A queue slice can only be used where a queue is expected, and an
// indexed part select of a queue is not supported.

module top;
  int q[$] = '{1, 2, 3};
  int x;

  initial begin
    x = q[0:1];
    x = q[1 +: 2];
    x = q[2 -: 2];
    $display("FAILED");
  end
endmodule
