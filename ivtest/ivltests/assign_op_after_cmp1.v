// Check that a assignment operator on an integer array entry with an immediate
// index works if it happes after a comparison that sets vvp flag 4 to 0.

module test;

  logic [7:0] x[10];
  logic a = 1'b0;

  initial begin
    x[0] = 10;
    if (a == 0) begin
      // Make sure that this update happens, even though the compare above
      // cleared set vvp flag 4
      x[0] += 1;
    end

    if (x[0] == 11) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected 11, got %0d", x[0]);
    end
  end

endmodule
