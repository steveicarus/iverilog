// Check that fork prefix labels work with all join types.

module test;

  reg failed;
  reg [2:0] value;

  initial begin
    failed = 1'b0;
    value = 3'b000;

    FORK_LABEL: (* keep = 1 *) fork
      value[0] = 1'b1;
    join : FORK_LABEL

    FORK_ANY_LABEL: (* keep = 1 *) fork
      value[1] = 1'b1;
    join_any : FORK_ANY_LABEL

    FORK_NONE_LABEL: (* keep = 1 *) fork
      value[2] = 1'b1;
    join_none : FORK_NONE_LABEL
    wait fork;

    if (value !== 3'b111) begin
      $display("FAILED(%0d). Fork prefix labels did not execute all blocks",
               `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
