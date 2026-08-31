module binary_xor_stack;
  integer failed;
  reg lhs1;
  reg rhs1;
  reg [63:0] lhs64;
  reg [63:0] rhs64;
  reg [64:0] lhs65;
  reg [64:0] rhs65;
  reg [64:0] third65;
  reg [256:0] lhs257;
  reg [256:0] rhs257;
  reg [256:0] third257;

  task check;
    input [256:0] actual;
    input [256:0] expected;
    input integer id;
    begin
      if (actual !== expected) begin
        $display("FAILED binary XOR stack test %0d", id);
        failed = failed + 1;
      end
    end
  endtask

  initial begin
    failed = 0;

    lhs1 = 1'b1;
    rhs1 = 1'b0;
    check(lhs1 ^ rhs1, 1'b1, 1);

    lhs64 = 64'h0123456789abcdef;
    rhs64 = 64'hfedcba9876543210;
    check(lhs64 ^ rhs64, 64'hffffffffffffffff, 2);

    // Exercise a multiword vector with both a single XOR and a chained
    // expression.
    lhs65 = {1'b1, 64'h0123456789abcdef};
    rhs65 = {1'b0, 64'hfedcba9876543210};
    third65 = {1'b1, 64'h0000000000000000};
    check(lhs65 ^ rhs65, {1'b1, 64'hffffffffffffffff}, 3);
    check((lhs65 ^ rhs65) ^ third65,
          {1'b0, 64'hffffffffffffffff}, 4);
    check(lhs65, {1'b1, 64'h0123456789abcdef}, 5);
    check(rhs65, {1'b0, 64'hfedcba9876543210}, 6);

    lhs65 = {4'b01xz, 61'b0};
    rhs65 = {4'b1010, 61'b0};
    check(lhs65 ^ rhs65, {4'b11xx, 61'b0}, 7);

    // Exercise several allocated words and both left- and right-nested XORs.
    lhs257 = {1'b1, {4{64'h0123456789abcdef}}};
    rhs257 = {1'b0, {4{64'hfedcba9876543210}}};
    third257 = {257{1'b1}};
    check(lhs257 ^ rhs257, {257{1'b1}}, 8);
    check((lhs257 ^ rhs257) ^ third257, {257{1'b0}}, 9);
    check(lhs257 ^ (rhs257 ^ third257), {257{1'b0}}, 10);
    check(lhs257, {1'b1, {4{64'h0123456789abcdef}}}, 11);
    check(rhs257, {1'b0, {4{64'hfedcba9876543210}}}, 12);

    if (failed == 0)
      $display("PASSED");
  end
endmodule
