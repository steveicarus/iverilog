// Check that a assignment operator on an real array entry with an immediate
// index works if it happes after a comparison that sets vvp flag 4 to 0.

module test;

  real r[1:0];
  logic a = 1'b0;

  initial begin
    r[0] = 8.0;
    if (a == 0) begin
      // Make sure that this update happens, even though the compare above
      // cleared set vvp flag 4
      r[0] *= 2.0;
    end

    if (r[0] == 16.0) begin
      $display("PASSED");
    end else begin
      $display("FAILED. Expected %f, got %f", 16.0, r[0]);
    end
  end

endmodule
