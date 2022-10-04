// Check that it is possible to copy an empty queue to an dynamic array.

module test;

  initial begin
    int d[];
    int q[$];
    d = '{1, 2, 3};
    d = q;
    if (d.size() == 0 && q.size() == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
