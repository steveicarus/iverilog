// Check that sub-blocks with variable declarations inside final procedures get
// executed


module test;

  final begin
    static int x = -1;
    for (int i = 0; i < 1; i++) begin
      x = i;
    end

    if (x === 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
