module wide_carry_arith;
  reg [64:0] a65;
  reg [64:0] b65;
  reg [64:0] result65;
  reg [129:0] a130;
  reg [129:0] b130;
  reg [129:0] result130;
  integer failed;

  task check65;
    input [64:0] actual;
    input [64:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED 65-bit test %0d: got %h, expected %h",
                 test_id, actual, expected);
        failed = failed + 1;
      end
    end
  endtask

  task check130;
    input [129:0] actual;
    input [129:0] expected;
    input integer test_id;
    begin
      if (actual !== expected) begin
        $display("FAILED 130-bit test %0d: got %h, expected %h",
                 test_id, actual, expected);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;

    a65 = 65'h0ffffffffffffffff;
    b65 = 65'h00000000000000001;
    result65 = a65 + b65;
    check65(result65, 65'h10000000000000000, 1);

    a65 = 65'h1ffffffffffffffff;
    result65 = a65 + b65;
    check65(result65, 65'h00000000000000000, 2);

    a130 = 130'h0ffffffffffffffffffffffffffffffff;
    b130 = 130'h000000000000000000000000000000001;
    result130 = a130 + b130;
    check130(result130, 130'h100000000000000000000000000000000, 3);

    a130 = 130'h00123456789abcdef0123456789abcdef;
    b130 = 130'h2fedcba98765432100123456789abcdef;
    result130 = a130 + b130;
    check130(result130, 130'h2ffffffffffffffff02468acf13579bde, 4);

    result130 = a130 - b130;
    check130(result130, 130'h102468acf13579bdf0000000000000000, 5);

    a130 = 130'h100000000000000000000000000000000;
    b130 = 130'h000000000000000000000000000000001;
    result130 = a130 - b130;
    check130(result130, 130'h0ffffffffffffffffffffffffffffffff, 6);

    a130 = 0;
    result130 = a130 - b130;
    check130(result130, 130'h3ffffffffffffffffffffffffffffffff, 7);

    a130 = 130'h00123456789abcdef0123456789abcdef;
    b130 = 130'h2fedcba98765432100123456789abcdef;
    result130 = a130 * b130;
    check130(result130, 130'h0fede05ff528828bddca5e20890f2a521, 8);

    b130[64] = 1'bx;
    result130 = a130 + b130;
    check130(result130, {130{1'bx}}, 9);
    result130 = a130 - b130;
    check130(result130, {130{1'bx}}, 10);
    result130 = a130 * b130;
    check130(result130, {130{1'bx}}, 11);

    b130[64] = 1'bz;
    result130 = a130 + b130;
    check130(result130, {130{1'bx}}, 12);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
