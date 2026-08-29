// Check that a later typedef does not hide an outer procedural l-value.

module test;

  reg failed;
  integer value;

  if (1) begin : inner
    initial begin
      failed = 1'b0;
      value = 42;

      if (value !== 42) begin
        $display("FAILED(%0d). Procedural l-value assignment failed",
                 `__LINE__);
        failed = 1'b1;
      end

      if (!failed) begin
        $display("PASSED");
      end
    end

    typedef logic value;
  end

endmodule
