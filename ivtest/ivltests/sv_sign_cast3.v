// Check that signed arguments to a sign cast are evaluated as self-determined.

module test;

  reg signed [3:0] op1;
  reg signed [2:0] op2;
  reg [7:0] result;
  bit failed = 1'b0;

  `define check(val, exp) \
    if (exp !== val) begin \
      $display("FAILED(%0d). Got %b, expected %b.", `__LINE__, val, exp); \
      failed = 1'b1; \
    end

  initial begin

    // Addition tests
    op1 = 4'b1111; op2 = 3'b111;
    result = 8'sd0 + signed'(op1 + op2);
    `check(result, 8'b11111110);
    result = 8'sd0 + unsigned'(op1 + op2);
    `check(result, 8'b00001110);

    op1 = 4'b1000; op2 = 3'b011;
    result = 8'sd0 + signed'(op1 + op2);
    `check(result, 8'b11111011);
    result = 8'sd0 + unsigned'(op1 + op2);
    `check(result, 8'b00001011);

    // Multiply tests
    op1 = 4'b0101; op2 = 3'b100;
    result = 8'sd0 + signed'(op1 * op2);
    `check(result, 8'b11111100);
    result = 8'sd0 + unsigned'(op1 * op2);
    `check(result, 8'b00001100);

    op1 = 4'b0010; op2 = 3'b100;
    result = 8'sd0 + signed'(op1 * op2);
    `check(result, 8'b11111000);
    result = 8'sd0 + unsigned'(op1 * op2);
    `check(result, 8'b00001000);

    // Left ASR tests
    op1 = 4'b1010;
    result = 8'sd0 + signed'(op1 <<< 1);
    `check(result, 8'b00000100);
    result = 8'sd0 + unsigned'(op1 <<< 1);
    `check(result, 8'b00000100);

    op1 = 4'b0101;
    result = 8'sd0 + signed'(op1 <<< 1);
    `check(result, 8'b11111010);
    result = 8'sd0 + unsigned'(op1 <<< 1);
    `check(result, 8'b00001010);

    // Right ASR tests
    op1 = 4'b0101;
    result = 8'sd0 + signed'(op1 >>> 1);
    `check(result, 8'b00000010);
    result = 8'sd0 + unsigned'(op1 >>> 1);
    `check(result, 8'b00000010);

    op1 = 4'b1010;
    result = 8'sd0 + signed'(op1 >>> 1);
    `check(result, 8'b11111101);
    result = 8'sd0 + unsigned'(op1 >>> 1);
    `check(result, 8'b00001101);

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
