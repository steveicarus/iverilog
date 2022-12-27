// Check that variables referenced in a void function contribute to the
// sensitivity list of a always_comb block.

module top;
  logic passed;
  logic [7:0] value;
  integer counter;

  function automatic void count(bit what);
    counter = 0;
    for (integer i = 0; i < $bits(value); i++) begin
      if (value[i] == what)
        counter += 1;
    end
  endfunction

  always_comb begin
    count(1'b1);
  end

  initial begin
    passed = 1'b1;

    value = 8'b0000_0000;
    #1;
    if (counter !== 0) begin
      $display("Expected 0, got %d", counter);
      passed = 1'b0;
    end

    value = 8'b0011_1100;
    #1;
    if (counter !== 4) begin
      $display("Expected 4, got %d", counter);
      passed = 1'b0;
    end

    value = 8'b1011_1101;
    #1;
    if (counter !== 6) begin
      $display("Expected 6, got %d", counter);
      passed = 1'b0;
    end

    if (passed) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
