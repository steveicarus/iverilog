// Check that blocking intra-assignment delays can assign real values.

module test;
  real r;
  reg failed;

  initial begin
    failed = 1'b0;

    r = #1 1.25;
    if (r != 1.25) begin
      $display("FAILED(%0d). Expected 1.25, got %0.2f", `__LINE__, r);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
