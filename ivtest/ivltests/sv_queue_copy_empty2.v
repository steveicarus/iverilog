// Check that it is possible to copy an empty dynamic array to a queue.

module test;

  initial begin
    int q[$];
    int d[];
    q = '{1, 2, 3};
    q = d;
    if (q.size() == 0 && d.size() == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
