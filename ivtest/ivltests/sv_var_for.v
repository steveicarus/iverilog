// Check that var keyword is supported in for loop variable declarations

module test;

  initial begin
    int j;
    for (var int i = 0; i < 10; i++) begin
      j = i;
    end
    if (j == 9) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
