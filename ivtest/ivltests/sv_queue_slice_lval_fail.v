// Assigning to a slice of a queue is not supported.

module top;
  int q[$] = '{1, 2, 3};

  initial begin
    q[0:1] = '{4, 5};
    $display("FAILED");
  end
endmodule
