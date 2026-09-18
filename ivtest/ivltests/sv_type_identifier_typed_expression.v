// Check that a later typedef does not hide an outer typed expression.

module test;

  integer value[$];

  initial value.push_back(42);

  if (1) begin : inner
    integer values[$];

    initial begin
      #1;
      values = value;

      if (values.size() != 1 || values[0] !== 42) begin
        $display("FAILED(%0d). Typed expression lookup failed", `__LINE__);
      end else begin
        $display("PASSED");
      end
    end

    typedef logic value;
  end

endmodule
