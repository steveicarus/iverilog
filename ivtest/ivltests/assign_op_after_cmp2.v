// Check that a assignment operator on a dynamic part select of an vector works
// if it happes after a comparison that sets vvp flag 4 to 0.

module test;

  logic [7:0] a = 8'h0;

  initial begin
    if (a == 0) begin
      // Make sure that this update happens, even though the compare above
      // cleared set vvp flag 4
      a[a+:1] += 1;
    end

    if (a == 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected 1, got %0d", a);
    end
  end

endmodule
