// Check that attributes on disabled assertions do not cause a crash.

module test;

  reg value;

  initial begin
    value = 1'b0;
    (* keep = 1 *) assert (0) else value = 1'b1;
    (* *) assert (0) else value = 1'b1;

    if (value === 1'b0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
