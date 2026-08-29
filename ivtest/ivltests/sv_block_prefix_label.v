// Check that sequential block prefix labels work.

module test;

  reg failed;
  reg value;

  initial begin
    failed = 1'b0;
    value = 1'b0;

    BLOCK_LABEL: (* keep = 1 *) begin
      value = 1'b1;
      disable BLOCK_LABEL;
      value = 1'b0;
    end : BLOCK_LABEL

    if (value !== 1'b1) begin
      $display("FAILED(%0d). Block prefix label did not create a named scope",
               `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
