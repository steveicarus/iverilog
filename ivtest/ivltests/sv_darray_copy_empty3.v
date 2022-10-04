// Check that it is possible to copy an empty dynamic array using a dynamic
// array new operation.

module test;

  initial begin
    int d1[];
    int d2[];
    d1 = '{1, 2, 3};
    d1 = new [2](d2);
    if (d1.size() == 2 && d2.size() == 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
