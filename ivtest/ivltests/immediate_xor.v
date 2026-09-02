module immediate_xor;
  reg lhs1;
  reg result1;
  reg [4:0] lhs5;
  reg [4:0] result5;
  reg [30:0] lhs31;
  reg [30:0] result31;
  reg [31:0] lhs32;
  reg [31:0] result32;
  reg [62:0] lhs63;
  reg [62:0] result63;
  reg [63:0] lhs64;
  reg [63:0] result64;
  integer failed;

  task check;
    input [63:0] actual;
    input [63:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED immediate XOR test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;

    lhs1 = 1'b1;
    result1 = lhs1 ^ 1'b1;
    check(result1, 1'b0, 1);

    lhs5 = 5'b10xz1;
    result5 = lhs5 ^ 5'b011x0;
    check(result5, 5'b11xx1, 2);

    lhs31 = 31'h12345678;
    result31 = lhs31 ^ 31'h76543210;
    check(result31, 31'h64606468, 3);

    lhs32 = 32'h89abcdef;
    result32 = lhs32 ^ 32'hfedcba98;
    check(result32, 32'h77777777, 4);

    lhs32 = 32'h01234567;
    result32 = lhs32 ^ 32'h00xz0001;
    check(result32, 32'h01xx4566, 5);

    lhs63 = 63'h123456789abcdef0;
    result63 = lhs63 ^ 63'h76543210fedcba98;
    check(result63, 63'h6460646864606468, 6);

    lhs64 = 64'h0123456789abcdef;
    result64 = lhs64 ^ 64'h0000000000000001;
    check(result64, 64'h0123456789abcdee, 7);

    // A literal that cannot fit in an immediate remains on the ordinary path.
    lhs64 = 64'h0123456789abcdef;
    result64 = lhs64 ^ 64'hfedcba9876543210;
    check(result64, 64'hffffffffffffffff, 8);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
