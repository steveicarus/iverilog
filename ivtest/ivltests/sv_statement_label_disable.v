// Check that statement labels can be used with disable statements.

module test;

  integer value;

  initial begin
    value = 0;

    L: if (1) begin
      value = 1;
      disable L;
      value = 2;
    end

    if (value === 1) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
