// Check that when the right hand operand of a shift operation
// contains 'x' or 'z' bits, the result is undefined.
module test;
  reg pass;
  reg signed [3:0] lhs;
  reg        [3:0] rhs;
  reg        [3:0] res;

  wire [3:0] res1 = lhs <<  rhs;
  wire [3:0] res2 = lhs >>  rhs;
  wire [3:0] res3 = lhs >>> rhs;

  wire [3:0] res4 = lhs <<  4'b000x;
  wire [3:0] res5 = lhs >>  4'b00x0;
  wire [3:0] res6 = lhs >>> 4'b0z00;

  wire [3:0] res7 = 4'd1 <<  4'b000x;
  wire [3:0] res8 = 4'd1 >>  4'b00x0;
  wire [3:0] res9 = 4'd1 >>> 4'b0z00;

  wire [3:0] res10 = 4'd0 <<  rhs;
  wire [3:0] res11 = 4'd0 >>  rhs;
  wire [3:0] res12 = 4'd0 >>> rhs;

  initial begin
    pass = 1'b1;
    lhs = 4'd1;

    if (res1 !== 4'bxxxx) begin
      $display("FAILED test 1, expected 4'bxxxx, got 4'b%b", res1);
      pass = 1'b0;
    end

    if (res2 !== 4'bxxxx) begin
      $display("FAILED test 2, expected 4'bxxxx, got 4'b%b", res2);
      pass = 1'b0;
    end

    if (res3 !== 4'bxxxx) begin
      $display("FAILED test 3, expected 4'bxxxx, got 4'b%b", res3);
      pass = 1'b0;
    end

    if (res4 !== 4'bxxxx) begin
      $display("FAILED test 4, expected 4'bxxxx, got 4'b%b", res4);
      pass = 1'b0;
    end

    if (res5 !== 4'bxxxx) begin
      $display("FAILED test 5, expected 4'bxxxx, got 4'b%b", res5);
      pass = 1'b0;
    end

    if (res6 !== 4'bxxxx) begin
      $display("FAILED test 6, expected 4'bxxxx, got 4'b%b", res6);
      pass = 1'b0;
    end

    if (res7 !== 4'bxxxx) begin
      $display("FAILED test 7, expected 4'bxxxx, got 4'b%b", res7);
      pass = 1'b0;
    end

    if (res8 !== 4'bxxxx) begin
      $display("FAILED test 8, expected 4'bxxxx, got 4'b%b", res8);
      pass = 1'b0;
    end

    if (res9 !== 4'bxxxx) begin
      $display("FAILED test 9, expected 4'bxxxx, got 4'b%b", res9);
      pass = 1'b0;
    end

    if (res10 !== 4'bxxxx) begin
      $display("FAILED test 10, expected 4'bxxxx, got 4'b%b", res10);
      pass = 1'b0;
    end

    if (res11 !== 4'bxxxx) begin
      $display("FAILED test 11, expected 4'bxxxx, got 4'b%b", res11);
      pass = 1'b0;
    end

    if (res12 !== 4'bxxxx) begin
      $display("FAILED test 12, expected 4'bxxxx, got 4'b%b", res12);
      pass = 1'b0;
    end

    res = lhs << rhs;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 13, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = lhs >> rhs;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 14, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = lhs >>> rhs;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 15, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = lhs << 4'b000x;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 16, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = lhs >> 4'b00x0;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 17, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = lhs >>> 4'b0z00;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 18, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = 4'd1 << 4'b000x;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 19, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = 4'd1 >> 4'b00x0;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 20, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = 4'd1 >>> 4'b0z00;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 21, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = 4'd0 << rhs;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 22, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = 4'd0 >> rhs;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 23, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    res = 4'd0 >>> rhs;
    if (res !== 4'bxxxx) begin
      $display("FAILED test 24, expected 4'bxxxx, got 4'b%b", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
