module top;
  reg passed;
  reg [4:1] result;

  initial begin
    passed = 1'b1;
    result = 4'b0000;
    // Fork some processes and wait for the one with the least delay to finish.
    fork
      #3 result[3] = 1'b1;
      #4 result[4] = 1'b1;
    join_none
    fork
      #1 result[1] = 1'b1;
      #2 result[2] = 1'b1;
    join_any
    // Disable the rest of the forked processes.
    disable fork;
    // Only the 1st bit should be set.
    if (result !== 4'b0001) begin
      $display("More than one process ran before the disable fork: %b", result);
      passed = 1'b0;
      result = 4'b0001;
    end
    // Wait to make sure the disabled processes do not run at a later time.
    #10;
    // Only the 1st bit should still be set.
    if (result !== 4'b0001) begin
      $display("Processes ran to completion after being disabled: %b", result);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
