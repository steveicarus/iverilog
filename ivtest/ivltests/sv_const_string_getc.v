module top;
  parameter string str = "Hello";
  parameter byte chr1 = str.getc(1);
  parameter byte chr5 = str.getc(5);
  logic passed;

  initial begin
    passed = 1'b1;

    if (chr1 !== 101) begin
      $display("FAILED: Expected 'e'(101) for index 1, got '%c'(%0d)", chr1, chr1);
      passed = 1'b0;
    end

    if (chr5 !== 0) begin
      $display("FAILED: Expected 0 for index 5, got %0d", chr5);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");
  end
endmodule
