// A negative packed range must not shift a negative mask.

module test;

  reg failed;
  reg [-2:0] a;

  initial begin
    failed = 1'b0;
    a = '0;
    a[-1] = 1'b1;
    a[-2] = 1'b1;
    if (a !== 3'b110) begin
      $display("FAILED: expected 110, got %b", a);
      failed = 1'b1;
    end
    if (!failed) $display("PASSED");
  end

endmodule
