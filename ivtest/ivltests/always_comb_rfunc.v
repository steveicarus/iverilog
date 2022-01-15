module top;
  logic passed;
  logic [7:0] value;
  integer ones;

  function automatic integer count_by_one(input integer start);
    if (start) count_by_one = (value[start] ? 1 : 0) + count_ones(start-1);
    else count_by_one = value[start] ? 1 : 0;
  endfunction

  function automatic integer count_ones(input integer start);
    if (start) count_ones = (value[start] ? 1 : 0) + count_by_one(start-1);
    else count_ones = value[start] ? 1 : 0;
  endfunction

  always_comb ones = count_ones(7);

  initial begin
    passed = 1'b1;

    value = 8'b0000_0000;
    #1;
    if (ones !== 0) begin
      $display("Expected 0, got %d", ones);
      passed = 1'b0;
    end

    value = 8'b0011_1100;
    #1;
    if (ones !== 4) begin
      $display("Expected 4, got %d", ones);
      passed = 1'b0;
    end

    value = 8'b1011_1101;
    #1;
    if (ones !== 6) begin
      $display("Expected 6, got %d", ones);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
