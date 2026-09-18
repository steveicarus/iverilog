// Check that attributes on a labeled assertion work with assertions disabled.

module test;

  reg value;

  initial begin
    value = 1'b0;
    L: (* keep = 1 *) assert (0) else value = 1'b1;

    if (value === 1'b0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
