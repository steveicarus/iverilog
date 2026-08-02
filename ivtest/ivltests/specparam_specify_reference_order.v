// Check that specify-block specparams can be referenced before declaration.

module test;

  specify
    specparam value = delay;
    specparam delay = 42;
  endspecify

  initial begin
    if (value !== 42) begin
      $display("FAILED. Expected 42, got %0d", value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
