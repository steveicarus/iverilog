// Check that an unchanged NaN stops propagating around a real feedback loop.

module test;

  real value;

  assign value = value / 0.0;

  initial begin
    #1;
    if (value != value) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected NaN, got %f", value);
    end
  end

endmodule
