// Check that it is possible to copy an empty queue to a dynamic array using a
// dynamic array new operation.

module test;

  initial begin
    int d[];
    int q[$];
    d = '{1, 2, 3};
    d = new [2](q);
    if (d.size() == 2 && q.size() == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
