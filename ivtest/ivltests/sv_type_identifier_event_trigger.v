// Check that a later typedef does not hide an outer named event.

module test;

  reg failed;
  reg seen;
  event value;

  always @value seen = 1'b1;

  if (1) begin : inner
    initial begin
      failed = 1'b0;
      seen = 1'b0;

      #1 -> value;
      #1;

      if (seen !== 1'b1) begin
        $display("FAILED(%0d). Named event was not triggered", `__LINE__);
        failed = 1'b1;
      end

      if (!failed) begin
        $display("PASSED");
      end
    end

    typedef logic value;
  end

endmodule
